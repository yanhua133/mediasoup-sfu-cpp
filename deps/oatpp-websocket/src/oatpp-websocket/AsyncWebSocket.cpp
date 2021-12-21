/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
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

#include "AsyncWebSocket.hpp"

#include "./Utils.hpp"

#if defined(WIN32) || defined(_WIN32)
  #include <WinSock2.h>
#else
  #include <arpa/inet.h>
#endif

namespace oatpp { namespace websocket {

AsyncWebSocket::AsyncWebSocket(const std::shared_ptr<oatpp::data::stream::IOStream>& connection, const Config& config)
  : m_config(config)
  , m_connection(connection)
  , m_listener(nullptr)
  , m_lastOpcode(-1)
  , m_listening(false)
{}

AsyncWebSocket::AsyncWebSocket(const std::shared_ptr<oatpp::data::stream::IOStream>& connection, bool maskOutgoingMessages)
  : m_connection(connection)
  , m_listener(nullptr)
  , m_lastOpcode(-1)
  , m_listening(false)
{
  m_config.maskOutgoingMessages = maskOutgoingMessages;
}

void AsyncWebSocket::setConfig(const Config& config) {
  m_config = config;
}

bool AsyncWebSocket::checkForContinuation(const Frame::Header& frameHeader) {
  if(m_lastOpcode == Frame::OPCODE_TEXT || m_lastOpcode == Frame::OPCODE_BINARY) {
    return false;
  }
  if(frameHeader.fin) {
    m_lastOpcode = -1;
  } else {
    m_lastOpcode = frameHeader.opcode;
  }
  return true;
}
  
oatpp::async::CoroutineStarter AsyncWebSocket::readFrameHeaderAsync(const std::shared_ptr<Frame::Header>& frameHeader) {
  
  class ReadFrameCoroutine : public oatpp::async::Coroutine<ReadFrameCoroutine> {
  private:
    std::shared_ptr<oatpp::data::stream::IOStream> m_connection;
    std::shared_ptr<Frame::Header> m_frameHeader;
  private:
    v_int32 m_lenType;
    v_uint16 m_bb;
    v_uint16 m_messageLen2;
    v_uint32 m_messageLen3 [2];
  private:
    oatpp::data::buffer::InlineReadData m_inlineData;
  public:
    
    ReadFrameCoroutine(const std::shared_ptr<oatpp::data::stream::IOStream> connection,
                       const std::shared_ptr<Frame::Header>& frameHeader)
      : m_connection(connection)
      , m_frameHeader(frameHeader)
    {
      m_inlineData.set(&m_bb, 2);
    }
    
    Action act() override {
      return m_connection->readExactSizeDataAsyncInline(m_inlineData, yieldTo(&ReadFrameCoroutine::onBbRead));
    }
    
    Action onBbRead() {
      
      v_uint8 messageLen1;
      Frame::unpackHeaderBits(ntohs(m_bb), *m_frameHeader, messageLen1);
      
      if(messageLen1 < 126) {
        m_lenType = 1;
        m_frameHeader->payloadLength = messageLen1;
        return yieldTo(&ReadFrameCoroutine::onLenRead);
      } else if(messageLen1 == 126) {
        m_lenType = 2;
        m_inlineData.set(&m_messageLen2, 2);
        return yieldTo(&ReadFrameCoroutine::readLen);
      } else if(messageLen1 == 127) {
        m_lenType = 3;
        m_inlineData.set(m_messageLen3, 8);
        return yieldTo(&ReadFrameCoroutine::readLen);
      }
      
      return error<Error>("[oatpp::web::protocol::websocket::AsyncWebSocket::readFrameHeaderAsync(){ReadFrameCoroutine}]: Invalid frame payload length.");
      
    }
    
    Action readLen() {
      return m_connection->readExactSizeDataAsyncInline(m_inlineData, yieldTo(&ReadFrameCoroutine::onLenRead));
    }
    
    Action onLenRead() {
      
      if(m_lenType == 2) {
        m_frameHeader->payloadLength = ntohs(m_messageLen2);
      } else if(m_lenType == 3) {
        m_frameHeader->payloadLength = (((v_uint64) ntohl(m_messageLen3[0])) << 32) | ntohl(m_messageLen3[1]);
      }
      
      if(m_frameHeader->hasMask) {
        m_inlineData.set(m_frameHeader->mask, 4);
        return yieldTo(&ReadFrameCoroutine::readMask);
      }
      
      return finish();
      
    }
    
    Action readMask() {
      return m_connection->readExactSizeDataAsyncInline(m_inlineData, finish());
    }
    
  };
  
  return ReadFrameCoroutine::start(m_connection, frameHeader);
  
}
  
oatpp::async::CoroutineStarter AsyncWebSocket::writeFrameHeaderAsync(const std::shared_ptr<Frame::Header>& frameHeader)
{
  
  class WriteFrameCoroutine : public oatpp::async::Coroutine<WriteFrameCoroutine> {
  private:
    std::shared_ptr<oatpp::data::stream::IOStream> m_connection;
    std::shared_ptr<Frame::Header> m_frameHeader;
  private:
    v_int32 m_lenType;
    v_uint16 m_bb;
    v_uint16 m_messageLen2;
    v_uint32 m_messageLen3 [2];
    v_uint8 m_messageLengthScenario;
  private:
    oatpp::data::buffer::InlineWriteData m_inlineData;
  public:
    
    WriteFrameCoroutine(const std::shared_ptr<oatpp::data::stream::IOStream> connection,
                        const std::shared_ptr<Frame::Header>& frameHeader)
      : m_connection(connection)
      , m_frameHeader(frameHeader)
    {
      ;
      Frame::packHeaderBits(m_bb, *m_frameHeader, m_messageLengthScenario);
      m_bb = htons(m_bb);
      m_inlineData.set(&m_bb, 2);
    }
    
    Action act() override {
      return m_connection->writeExactSizeDataAsyncInline(m_inlineData, yieldTo(&WriteFrameCoroutine::onBbWritten));
    }
    
    Action onBbWritten() {
      if(m_messageLengthScenario == 2) {
        m_messageLen2 = htons(m_frameHeader->payloadLength);
        m_inlineData.set(&m_messageLen2, 2);
        return yieldTo(&WriteFrameCoroutine::writeMessageLen);
      } else if(m_messageLengthScenario == 3) {
        m_messageLen3[0] = htonl(m_frameHeader->payloadLength >> 32);
        m_messageLen3[1] = htonl(m_frameHeader->payloadLength & 0xFFFFFFFF);
        m_inlineData.set(m_messageLen3, 8);
        return yieldTo(&WriteFrameCoroutine::writeMessageLen);
      }
      return yieldTo(&WriteFrameCoroutine::onLenWritten);
    }
    
    Action writeMessageLen() {
      return m_connection->writeExactSizeDataAsyncInline(m_inlineData, yieldTo(&WriteFrameCoroutine::onLenWritten));
    }
    
    Action onLenWritten() {
      if(m_frameHeader->hasMask) {
        m_inlineData.set(m_frameHeader->mask, 4);
        return yieldTo(&WriteFrameCoroutine::writeMask);
      }
      return finish();
    }
    
    Action writeMask() {
      return m_connection->writeExactSizeDataAsyncInline(m_inlineData, finish());
    }
    
  };
  
  return WriteFrameCoroutine::start(m_connection, frameHeader);
  
}

oatpp::async::CoroutineStarter AsyncWebSocket::readPayloadAsync(const std::shared_ptr<Frame::Header>& frameHeader,
                                                                const std::shared_ptr<oatpp::data::stream::ChunkedBuffer>& shortMessageStream)
{
  
  class ReadPayloadCoroutine : public oatpp::async::Coroutine<ReadPayloadCoroutine> {
  private:
    std::shared_ptr<AsyncWebSocket> m_socket;
    std::shared_ptr<oatpp::data::stream::IOStream> m_connection;
    std::shared_ptr<Frame::Header> m_frameHeader;
    std::shared_ptr<oatpp::data::stream::ChunkedBuffer> m_shortMessageStream;
    std::shared_ptr<Listener> m_listener;
  private:
    p_char8 m_buffer;
    oatpp::v_io_size m_progress;
  private:
    oatpp::data::buffer::InlineReadData m_inlineData;
  public:
    ReadPayloadCoroutine(const std::shared_ptr<AsyncWebSocket>& socket,
                         const std::shared_ptr<oatpp::data::stream::IOStream>& connection,
                         const std::shared_ptr<Frame::Header>& frameHeader,
                         const std::shared_ptr<oatpp::data::stream::ChunkedBuffer>& shortMessageStream,
                         const std::shared_ptr<Listener>& listener)
      : m_socket(socket)
      , m_connection(connection)
      , m_frameHeader(frameHeader)
      , m_shortMessageStream(shortMessageStream)
      , m_listener(listener)
      , m_buffer(new v_char8[m_socket->m_config.readBufferSize])
      , m_progress(0)
    {
      if(m_shortMessageStream && m_frameHeader->payloadLength > 125) {
        throw std::runtime_error("[oatpp::web::protocol::websocket::AsyncWebSocket::readPayloadAsync()]: Invalid payloadLength. See RFC-6455, section-5.5.");
      }
    }
    
    ~ReadPayloadCoroutine() {
      delete [] m_buffer;
    }
    
    Action act() {
      
      if(m_progress < m_frameHeader->payloadLength) {
        
        oatpp::v_io_size desiredSize = m_socket->m_config.readBufferSize;
        if(desiredSize > m_frameHeader->payloadLength - m_progress) {
          desiredSize = m_frameHeader->payloadLength - m_progress;
        }
        m_inlineData.set(m_buffer, desiredSize);
        return yieldTo(&ReadPayloadCoroutine::readData);
        
      }
      
      if(m_shortMessageStream == nullptr && m_frameHeader->fin && m_listener) {
        return m_listener->readMessage(m_socket, m_frameHeader->opcode, nullptr, 0).next(finish());
      }
      
      return finish();
      
    }
    
    Action readData() {
      return m_connection->readSomeDataAsyncInline(m_inlineData, yieldTo(&ReadPayloadCoroutine::onDataRead));
    }
    
    Action onDataRead() {
      
      auto readResult = (v_int64) m_inlineData.currBufferPtr - (v_int64)m_buffer;
      
      if(readResult > 0) {
      
        if(m_frameHeader->hasMask) {
          std::unique_ptr<v_char8[]> decoded(new v_char8[readResult]);
          for(v_int32 i = 0; i < readResult; i ++) {
            decoded.get()[i] = m_buffer[i] ^ m_frameHeader->mask[(i + m_progress) % 4];
          }
          
          m_progress += readResult;
          
          if(m_shortMessageStream) {
            /* this is RAM stream. Non-blocking call */
            m_shortMessageStream->writeSimple(decoded.get(), readResult);
          } else if(m_listener) {
            return m_listener->readMessage(m_socket, m_frameHeader->opcode, decoded.get(), readResult).next(yieldTo(&ReadPayloadCoroutine::act));
          }
          
        } else {
          
          m_progress += readResult;
          
          if(m_shortMessageStream) {
            /* this is RAM stream. Non-blocking call */
            m_shortMessageStream->writeSimple(m_buffer, readResult);
          } else if(m_listener) {
            return m_listener->readMessage(m_socket, m_frameHeader->opcode, m_buffer, readResult).next(yieldTo(&ReadPayloadCoroutine::act));
          }
          
        }
        
        return yieldTo(&ReadPayloadCoroutine::act);
        
      }
      
      throw std::runtime_error("[oatpp::web::protocol::websocket::AsyncWebSocket::readPayloadAsync(){ReadPayloadCoroutine}]: Invalid connection state.");
      
    }
    
  };
  
  if(shortMessageStream && frameHeader->payloadLength > 125) {
    throw std::runtime_error("[oatpp::web::protocol::websocket::AsyncWebSocket::readPayloadAsync()]: Invalid frame payload length. See RFC-6455, section-5.5.");
  }

  return ReadPayloadCoroutine::start(shared_from_this(), m_connection, frameHeader, shortMessageStream, m_listener);
  
}
  
oatpp::async::CoroutineStarter AsyncWebSocket::handleFrameAsync(const std::shared_ptr<Frame::Header>& frameHeader) {
  
  class HandleFrameCoroutine : public oatpp::async::Coroutine<HandleFrameCoroutine> {
  private:
    std::shared_ptr<AsyncWebSocket> m_socket;
    std::shared_ptr<Frame::Header> m_frameHeader;
    std::shared_ptr<Listener> m_listener;
  private:
    std::shared_ptr<oatpp::data::stream::ChunkedBuffer> m_shortMessageStream;
  public:
    HandleFrameCoroutine(const std::shared_ptr<AsyncWebSocket>& socket,
                         const std::shared_ptr<Frame::Header>& frameHeader)
      : m_socket(socket)
      , m_frameHeader(frameHeader)
      , m_listener(socket->m_listener)
    {}
    
    Action act() override {
      
      switch (m_frameHeader->opcode) {
        case Frame::OPCODE_CONTINUATION:
          if(m_socket->m_lastOpcode < 0) {
            throw std::runtime_error("[oatpp::web::protocol::websocket::AsyncWebSocket::handleFrameAsync(){HandleFrameCoroutine}]: Invalid communication state.");
          }
          return m_socket->readPayloadAsync(m_frameHeader, nullptr).next(finish());
          
        case Frame::OPCODE_TEXT:
          if(m_socket->checkForContinuation(*m_frameHeader)) {
            return m_socket->readPayloadAsync(m_frameHeader, nullptr).next(finish());
          } else {
            throw std::runtime_error("[oatpp::web::protocol::websocket::AsyncWebSocket::handleFrameAsync(){HandleFrameCoroutine}]: Invalid communication state. OPCODE_CONTINUATION expected");
          }
          
        case Frame::OPCODE_BINARY:
          if(m_socket->checkForContinuation(*m_frameHeader)) {
            return m_socket->readPayloadAsync(m_frameHeader, nullptr).next(finish());
          } else {
            throw std::runtime_error("[oatpp::web::protocol::websocket::AsyncWebSocket::handleFrameAsync(){HandleFrameCoroutine}]: Invalid communication state. OPCODE_CONTINUATION expected");
          }
          
        case Frame::OPCODE_CLOSE:
          m_shortMessageStream = oatpp::data::stream::ChunkedBuffer::createShared();
          return m_socket->readPayloadAsync(m_frameHeader, m_shortMessageStream).next(yieldTo(&HandleFrameCoroutine::onClose));
          
        case Frame::OPCODE_PING:
          m_shortMessageStream = oatpp::data::stream::ChunkedBuffer::createShared();
          return m_socket->readPayloadAsync(m_frameHeader, m_shortMessageStream).next(yieldTo(&HandleFrameCoroutine::onPing));
          
        case Frame::OPCODE_PONG:
          m_shortMessageStream = oatpp::data::stream::ChunkedBuffer::createShared();
          return m_socket->readPayloadAsync(m_frameHeader, m_shortMessageStream).next(yieldTo(&HandleFrameCoroutine::onPong));
          
        default:
          throw std::runtime_error("[oatpp::web::protocol::websocket::AsyncWebSocket::handleFrameAsync(){HandleFrameCoroutine}]: Unknown frame");
          break;
      }
      
    }
    
    Action onClose() {
      if(m_listener) {
        v_uint16 code = 0;
        oatpp::String message;
        if(m_shortMessageStream->getSize() >= 2) {
          m_shortMessageStream->readSubstring(&code, 0, 2);
          code = ntohs(code);
          message = m_shortMessageStream->getSubstring(2, m_shortMessageStream->getSize() - 2);
        }
        if(!message) {
          message = "";
        }
        return m_listener->onClose(m_socket, code, message).next(finish());
      }
      return finish();
    }
    
    Action onPing() {
      if(m_listener) {
        return m_listener->onPing(m_socket, m_shortMessageStream->toString()).next(finish());
      }
      return finish();
    }
    
    Action onPong() {
      if(m_listener) {
        return m_listener->onPong(m_socket, m_shortMessageStream->toString()).next(finish());
      }
      return finish();
    }
    
  };
  
  return HandleFrameCoroutine::start(shared_from_this(), frameHeader);
  
}
  
oatpp::async::CoroutineStarter AsyncWebSocket::listenAsync() {

  class ListenCoroutine : public oatpp::async::Coroutine<ListenCoroutine> {
  private:
    std::shared_ptr<AsyncWebSocket> m_socket;
    std::shared_ptr<Frame::Header> m_frameHeader;
  public:
    
    ListenCoroutine(const std::shared_ptr<AsyncWebSocket>& socket)
      : m_socket(socket)
      , m_frameHeader(std::make_shared<Frame::Header>())
    {
      m_frameHeader->opcode = -1;
    }
    
    Action act() override {
      if(m_frameHeader->opcode != Frame::OPCODE_CLOSE ) {
        return m_socket->readFrameHeaderAsync(m_frameHeader).next(m_socket->handleFrameAsync(m_frameHeader)).next(yieldTo(&ListenCoroutine::act));
      }
      return finish();
    }
    
    Action handleError(Error* error) override {
      return error;
    }
    
  };
  
  return ListenCoroutine::start(shared_from_this());
  
}
  
oatpp::async::CoroutineStarter AsyncWebSocket::sendFrameHeaderAsync(const std::shared_ptr<Frame::Header>& frameHeader,
                                                                    bool fin, v_uint8 opcode, v_int64 messageSize)
{

  frameHeader->fin = fin;
  frameHeader->rsv1 = false;
  frameHeader->rsv2 = false;
  frameHeader->rsv3 = false;
  frameHeader->opcode = opcode;
  frameHeader->hasMask = m_config.maskOutgoingMessages;
  frameHeader->payloadLength = messageSize;

  if(frameHeader->hasMask) {
    Utils::generateMaskForFrame(*frameHeader);
  }

  return writeFrameHeaderAsync(frameHeader);
}
  
oatpp::async::CoroutineStarter AsyncWebSocket::sendOneFrameAsync(bool fin, v_uint8 opcode, const oatpp::String& message) {
  
  class SendFrameCoroutine : public oatpp::async::Coroutine<SendFrameCoroutine> {
  private:
    std::shared_ptr<AsyncWebSocket> m_socket;
    bool m_fin;
    v_uint8 m_opcode;
    oatpp::String m_message;
    std::shared_ptr<Frame::Header> m_frameHeader;
  private:
    p_char8 m_encoded = nullptr;
  private:
    oatpp::data::buffer::InlineWriteData m_inlineData;
  public:
    SendFrameCoroutine(const std::shared_ptr<AsyncWebSocket>& socket,
                       bool fin, v_uint8 opcode, const oatpp::String& message)
      : m_socket(socket)
      , m_fin(fin)
      , m_opcode(opcode)
      , m_message(message)
      , m_frameHeader(std::make_shared<Frame::Header>())
    {}
    
    ~SendFrameCoroutine() {
      if(m_encoded != nullptr) {
        delete [] m_encoded;
      }
    }
    
    Action act() override {
      if(m_message && m_message->getSize() > 0) {
        return m_socket->sendFrameHeaderAsync(m_frameHeader, m_fin, m_opcode, m_message->getSize()).next(yieldTo(&SendFrameCoroutine::prepareWriteMessage));
      } else {
        return m_socket->sendFrameHeaderAsync(m_frameHeader, m_fin, m_opcode, 0).next(finish());
      }
    }
    
    Action prepareWriteMessage() {
      if(m_frameHeader->hasMask) {
        m_encoded = new v_char8[m_message->getSize()];
        for(v_int32 i = 0; i < m_message->getSize(); i ++) {
          m_encoded[i] = m_message->getData()[i] ^ m_frameHeader->mask[i % 4];
        }
        m_inlineData.set(m_encoded, m_message->getSize());
      } else {
        m_inlineData.set(m_message->getData(), m_message->getSize());
      }
      return yieldTo(&SendFrameCoroutine::writeMessage);
    }
    
    Action writeMessage() {
      return m_socket->m_connection->writeExactSizeDataAsyncInline(m_inlineData, finish());
    }
    
  };
  
  return SendFrameCoroutine::start(shared_from_this(), fin, opcode, message);
  
}
  
oatpp::async::CoroutineStarter AsyncWebSocket::sendCloseAsync(v_uint16 code, const oatpp::String& message) {

  code = htons(code);
  
  oatpp::data::stream::ChunkedBuffer buffer;
  buffer.writeSimple(&code, 2);
  if(message) {
    buffer.writeSimple(message->getData(), message->getSize());
  }
  
  return sendOneFrameAsync(true, Frame::OPCODE_CLOSE, buffer.toString());
  
}

oatpp::async::CoroutineStarter AsyncWebSocket::sendCloseAsync() {
  return sendOneFrameAsync(true, Frame::OPCODE_CLOSE, nullptr);
}

oatpp::async::CoroutineStarter AsyncWebSocket::sendPingAsync(const oatpp::String& message) {
  return sendOneFrameAsync(true, Frame::OPCODE_PING, message);
}

oatpp::async::CoroutineStarter AsyncWebSocket::sendPongAsync(const oatpp::String& message) {
  return sendOneFrameAsync(true, Frame::OPCODE_PONG, message);
}

oatpp::async::CoroutineStarter AsyncWebSocket::sendOneFrameTextAsync(const oatpp::String& message) {
  return sendOneFrameAsync(true, Frame::OPCODE_TEXT, message);
}

oatpp::async::CoroutineStarter AsyncWebSocket::sendOneFrameBinaryAsync(const oatpp::String& message) {
  return sendOneFrameAsync(true, Frame::OPCODE_BINARY, message);
}
  
}}
