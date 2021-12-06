/***************************************************************************
 *
 * Project:   ______                ______ _
 *           / _____)              / _____) |          _
 *          | /      ____ ____ ___| /     | | _   ____| |_
 *          | |     / _  |  _ (___) |     | || \ / _  |  _)
 *          | \____( ( | | | | |  | \_____| | | ( ( | | |__
 *           \______)_||_|_| |_|   \______)_| |_|\_||_|\___)
 *
 *
 * Copyright 2020-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#include "Message.h"
#include "Peer.hpp"
#include "Room.hpp"

#include "oatpp/network/tcp/Connection.hpp"
#include "oatpp/encoding/Base64.hpp"


void Peer::sendMessageAsync(json message) {

  class SendMessageCoroutine : public oatpp::async::Coroutine<SendMessageCoroutine> {
  private:
    oatpp::async::Lock* m_lock;
    std::shared_ptr<AsyncWebSocket> m_websocket;
    oatpp::String m_message;
  public:

    SendMessageCoroutine(oatpp::async::Lock* lock,
                         const std::shared_ptr<AsyncWebSocket>& websocket,
                         const oatpp::String& message)
      : m_lock(lock)
      , m_websocket(websocket)
      , m_message(message)
    {}

    Action act() override {
      return oatpp::async::synchronize(m_lock, m_websocket->sendOneFrameTextAsync(m_message)).next(finish());
    }

  };

  if(m_socket) {
    m_asyncExecutor->execute<SendMessageCoroutine>(&m_writeLock, m_socket, oatpp::String(message.dump().c_str()));
  }

}

oatpp::async::Action Peer::checkResponseAsync(int messageId, string method, string param, oatpp::async::Action&& nextAction) {

    auto sent_element = this->m_sents.find(messageId);
    if (sent_element != m_sents.end()) {
           //需要重复查找,直到找不到为止,好像还得有个超时机制
        return oatpp::async::Action::createActionByType(oatpp::async::Action::TYPE_REPEAT);
    }else {
        if (method == "newConsumer" && param != "") {
            auto& consumer = this->data.consumers[param];
            consumer->resume();
        }
        return std::forward<oatpp::async::Action>(nextAction);
    }
}

void Peer::requestAsync(std::string method, json message) {
    class RequestCoroutine : public oatpp::async::Coroutine<RequestCoroutine> {
    private:
        std::shared_ptr<Peer> m_peer;
        oatpp::async::Lock* m_lock;
        std::shared_ptr<AsyncWebSocket> m_websocket;
        json m_message;
    public:
        
        RequestCoroutine(const std::shared_ptr<Peer>& peer,
                         oatpp::async::Lock* lock,
                         const std::shared_ptr<AsyncWebSocket>& websocket,
                         const json& message)
        : m_peer(peer)
        , m_lock(lock)
        , m_websocket(websocket)
        , m_message(message)
        {}
        
        Action act() override {
            auto sentMsg = oatpp::String(m_message.dump().c_str());
            m_peer->m_sents[m_message["id"].get<int>()] = m_message;
            return oatpp::async::synchronize(m_lock, m_websocket->sendOneFrameTextAsync(sentMsg)).next(yieldTo(&RequestCoroutine::waitResponse));
        }
        
        Action waitResponse() {
            auto messageId = m_message["id"].get<int>();
            auto method = m_message["method"].get<std::string>();
            if (method == "newConsumer") {
                auto data = m_message["data"];
                auto consumerId = data["id"].get<std::string>();
                return m_peer->checkResponseAsync(messageId, method, consumerId, finish());
            }
            else {
                return m_peer->checkResponseAsync(messageId, "", "", finish());
            }
            //return waitFor(std::chrono::milliseconds(100)).next(finish());
        }
        
        //Action onMessageResponse() {
        //    //判断sent map里面是否有该值
        //    //结束coroutine
        //    finish();
        //}
        
    };
    
    if(m_socket) {
        auto request = Message::createRequest(method, message);
        m_asyncExecutor->execute<RequestCoroutine>(shared_from_this(), &m_writeLock, m_socket, request);
    }
    
    
}

void Peer::notifyAsync(std::string method, json message) {
  class NotifyCoroutine : public oatpp::async::Coroutine<NotifyCoroutine> {
  private:
    oatpp::async::Lock* m_lock;
    std::shared_ptr<AsyncWebSocket> m_websocket;
    oatpp::String m_message;
  public:

    NotifyCoroutine(oatpp::async::Lock* lock,
                         const std::shared_ptr<AsyncWebSocket>& websocket,
                         const oatpp::String& message)
      : m_lock(lock)
      , m_websocket(websocket)
      , m_message(message)
    {}

    Action act() override {
      return oatpp::async::synchronize(m_lock, m_websocket->sendOneFrameTextAsync(m_message)).next(finish());
    }

  };

  if(m_socket) {
      auto notify = Message::createNotification(method, message);
      std::cout << "[Room] [Peer] notifyAsync notify=" << notify.dump(4) << endl;
    m_asyncExecutor->execute<NotifyCoroutine>(&m_writeLock, m_socket, oatpp::String(notify.dump().c_str()));
  }    
}

void Peer::handleRequest(json request){
    MS_lOGD("[Room] [Peer] handleRequest peerId=%s,request=%s",this->m_peerId.c_str(), request.dump(4).c_str());
    std::function<void(json data)> accept([&, request](json data)
                                          {
        auto response = Message::createSuccessResponse(request, data);
        std::cout << "[Room] [Peer] handleRequest with accept response=" << response.dump(4) << endl;
        sendMessageAsync(response);
    });
    //需要注意的是，capture by reference是不会修改被capture变量的生命周期的，你要保证被capture的变量在closure运行时是有效的。
    std::function<void(int errorCode, std::string errorReason)> reject([&, request](int errorCode, std::string errorReason)
                                                                       {
        auto response = Message::createErrorResponse(request, errorCode, errorReason);
        std::cout << "[Room] [Peer] handleRequest with reject response=" << response.dump(4) << endl;
        sendMessageAsync(response);
    });
    //auto shared_peer = std::make_shared<Peer>((Peer*)this);
    auto shared_this = shared_from_this();
    m_room->handleRequest(shared_this, request, accept, reject);
}

void Peer::handleResponse(json response){
    auto sent_element = this->m_sents.find(response["id"].get<int>());
    if (sent_element == m_sents.end()) {
        std::cout << "[Peer] response id not found in map!\n" << std::endl;
        return;
    }
    
    if (response.contains("ok"))
    {
        if (response["ok"].get<bool>())
            this->m_sents.erase(sent_element);
    }
    else
    {
        auto error = "error response!";
        this->m_sents.erase(sent_element);
    }
}

void Peer::handleNotification(json notification){
  m_room->handleNotification(notification);
}

bool Peer::sendPingAsync() {

  class SendPingCoroutine : public oatpp::async::Coroutine<SendPingCoroutine> {
  private:
    oatpp::async::Lock* m_lock;
    std::shared_ptr<AsyncWebSocket> m_websocket;
  public:

    SendPingCoroutine(oatpp::async::Lock* lock, const std::shared_ptr<AsyncWebSocket>& websocket)
      : m_lock(lock)
      , m_websocket(websocket)
    {}

    Action act() override {
      return oatpp::async::synchronize(m_lock, m_websocket->sendPingAsync(nullptr)).next(finish());
    }

  };

  /******************************************************
   *
   * Ping counter is increased on sending ping
   * and decreased on receiving pong from the client.
   *
   * If the server didn't receive pong from client
   * before the next ping,- then the client is
   * considered to be disconnected.
   *
   ******************************************************/

  ++ m_pingPoingCounter;

  if(m_socket && m_pingPoingCounter == 1) {
    m_asyncExecutor->execute<SendPingCoroutine>(&m_writeLock, m_socket);
    return true;
  }

  return false;

}

oatpp::async::CoroutineStarter Peer::onApiError(const oatpp::String& errorMessage) {

  class SendErrorCoroutine : public oatpp::async::Coroutine<SendErrorCoroutine> {
  private:
    oatpp::async::Lock* m_lock;
    std::shared_ptr<AsyncWebSocket> m_websocket;
    oatpp::String m_message;
  public:

    SendErrorCoroutine(oatpp::async::Lock* lock,
                       const std::shared_ptr<AsyncWebSocket>& websocket,
                       const oatpp::String& message)
      : m_lock(lock)
      , m_websocket(websocket)
      , m_message(message)
    {}

    Action act() override {
      /* synchronized async pipeline */
      return oatpp::async::synchronize(
        /* Async write-lock to prevent concurrent writes to socket */
        m_lock,
        /* send error message, then close-frame */
        std::move(m_websocket->sendOneFrameTextAsync(m_message).next(m_websocket->sendCloseAsync()))
      ).next(
        /* async error after error message and close-frame are sent */
        new oatpp::async::Error("API Error")
      );
    }

  };

  auto message = MessageDto::createShared();
  message->code = MessageCodes::CODE_API_ERROR;
  message->message = errorMessage;

  return SendErrorCoroutine::start(&m_writeLock, m_socket, m_objectMapper->writeToString(message));

}

oatpp::async::CoroutineStarter Peer::handleMessage(const json message) {
    MS_lOGD("[Room] [Peer] handleMessage message=%s",message.dump(4).c_str());
    if (message.contains("request")){
        if(message["request"].is_boolean()){
            this->handleRequest(message);
        }
    }
    else if (message.contains("response")){
        if(message["response"].is_boolean()){
            this->handleResponse(message);
        }
    }
    else if(message.contains("notification")){
        if(message["notification"].is_boolean()){
            this->handleNotification(message);
        }
    }
//  if(!message->code) {
//    return onApiError("No message code provided.");
//  }
//
//  switch(*message->code) {
//
//    case MessageCodes::CODE_PEER_MESSAGE:
//      m_room->addHistoryMessage(message);
//      m_room->sendMessageAsync(message);
//      ++ m_statistics->EVENT_PEER_SEND_MESSAGE;
//      break;
//
//    case MessageCodes::CODE_PEER_IS_TYPING:
//      m_room->sendMessageAsync(message); break;
//
//    default:
//      return onApiError("Invalid client message code.");

  //}
    return nullptr;// do nothing

}

std::shared_ptr<Room> Peer::getRoom() {
  return m_room;
}

std::string Peer::getNickname() {
  return m_nickname;
}

void Peer::setNickname(std::string nickName){
    m_nickname = nickName;
}

std::string Peer::getPeerId() {
  return m_peerId;
}

void Peer::invalidateSocket() {
  if(m_socket) {
    m_serverConnectionProvider->invalidate(m_socket->getConnection());
  }
  m_socket.reset();
}

oatpp::async::CoroutineStarter Peer::onPing(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) {
  return oatpp::async::synchronize(&m_writeLock, socket->sendPongAsync(message));
}

oatpp::async::CoroutineStarter Peer::onPong(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) {
  -- m_pingPoingCounter;
  return nullptr; // do nothing
}

oatpp::async::CoroutineStarter Peer::onClose(const std::shared_ptr<AsyncWebSocket>& socket, v_uint16 code, const oatpp::String& message) {
  return nullptr; // do nothing
}

oatpp::async::CoroutineStarter Peer::readMessage(const std::shared_ptr<AsyncWebSocket>& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) {

  if(m_messageBuffer.getSize() + size >  m_appConfig->maxMessageSizeBytes) {
    return onApiError("Message size exceeds max allowed size.");
  }

  if(size == 0) { // message transfer finished

    auto wholeMessage = m_messageBuffer.toString();
    m_messageBuffer.clear();
      MS_lOGD("[Room] [Peer] readMessage=%s",wholeMessage->c_str());
//    oatpp::Object<MessageDto> message;
//
//    try {
//      message = m_objectMapper->readFromString<oatpp::Object<MessageDto>>(wholeMessage);
//    } catch (const std::runtime_error& e) {
//      return onApiError("Can't parse message");
//    }
//
//    message->peerName = m_nickname;
//    message->peerId = m_peerId;
//    message->timestamp = oatpp::base::Environment::getMicroTickCount();

      return handleMessage(Message::parse(wholeMessage->std_str() ));

  } else if(size > 0) { // message frame received
    m_messageBuffer.writeSimple(data, size);
  }

  return nullptr; // do nothing

}
