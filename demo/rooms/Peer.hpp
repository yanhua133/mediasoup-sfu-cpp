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

#ifndef ASYNC_SERVER_ROOMS_PEER_HPP
#define ASYNC_SERVER_ROOMS_PEER_HPP

#include "../dto/DTOs.hpp"
#include "../dto/ConfigDto.hpp"
#include "../utils/Statistics.hpp"

#include "oatpp-websocket/AsyncWebSocket.hpp"

#include "oatpp/network/ConnectionProvider.hpp"

#include "oatpp/core/async/Lock.hpp"
#include "oatpp/core/async/Executor.hpp"
#include "oatpp/core/data/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/component.hpp"

#include "json.hpp"
#include "Consumer.hpp"
#include "Producer.hpp"
#include "DataProducer.hpp"
#include "DataConsumer.hpp"
#include "Transport.hpp"

class Room; // FWD

struct _device
{
    std::string flag ;//   : "broadcaster",
    std::string name ;//   : device.name || "Unknown device",
    std::string version ;//: device.version
};

struct PeerInfo
{
    std::string id;
    std::string displayName;
    _device device;
    json producerInfo;
    std::vector<json> producers;
 
};

class PeerData
{
  public:
  std::string id;

  bool consume{ true };
  bool joined{ false };
  std::string displayName;
  json device;
  json rtpCapabilities;
  json sctpCapabilities;

  std::unordered_map<std::string,std::shared_ptr<Transport> > transports;
  std::unordered_map<std::string,std::shared_ptr<Producer> > producers;
  std::unordered_map<std::string,std::shared_ptr<Consumer> > consumers;
  std::unordered_map<std::string,std::shared_ptr<DataProducer> > dataProducers;
  std::unordered_map<std::string,std::shared_ptr<DataConsumer> > dataConsumers;
};

class Peer : public oatpp::websocket::AsyncWebSocket::Listener, public std::enable_shared_from_this<Peer> {
  public:
  PeerData data;
private:

  /**
   * Buffer for messages. Needed for multi-frame messages.
   */
  oatpp::data::stream::ChunkedBuffer m_messageBuffer;

  /**
   * Lock for synchronization of writes to the web socket.
   */
  oatpp::async::Lock m_writeLock;

private:
  std::shared_ptr<AsyncWebSocket> m_socket;
  std::shared_ptr<Room> m_room;
  std::string m_nickname;
  std::string m_peerId;
private:
  std::atomic<v_int32> m_pingPoingCounter;
private:

  /* Inject application components */

  OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, m_asyncExecutor);
  OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, m_objectMapper);
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, m_serverConnectionProvider);
  OATPP_COMPONENT(oatpp::Object<ConfigDto>, m_appConfig);
  OATPP_COMPONENT(std::shared_ptr<Statistics>, m_statistics);

private:

  oatpp::async::CoroutineStarter onApiError(const oatpp::String& errorMessage);

private:
    
  oatpp::async::CoroutineStarter handleMessage(const json message);

public:

  Peer(const std::shared_ptr<AsyncWebSocket>& socket,
       const std::shared_ptr<Room>& room,
       const std::string peerId,
       const std::string nickname
       )
    : m_socket(socket)
    , m_room(room)
    , m_peerId(peerId)
    , m_nickname(nickname)
    , m_pingPoingCounter(0)
  {}

  /**
   * Send message to peer (to user).
   * @param message
   */
  //void sendMessageAsync(const oatpp::Object<MessageDto>& message);
    void sendMessageAsync(json message);
  void requestAsync(std::string method, json message);
  void notifyAsync(std::string method, json message);
    oatpp::async::Action checkResponseAsync(int messageId, std::string method, std::string param, oatpp::async::Action&& nextAction);

  /**
   * Send Websocket-Ping.
   * @return - `true` - ping was sent.
   * `false` peer has not responded to the last ping, it means we have to disconnect him.
   */
  bool sendPingAsync();

  /**
   * Get room of the peer.
   * @return
   */
  std::shared_ptr<Room> getRoom();

  /**
   * Get peer nickname.
   * @return
   */
  std::string getNickname();
   
    //set peer nickname
  void setNickname(std::string nickname);

  /**
   * Get peer peerId.
   * @return
   */
  std::string getPeerId();

  /**
   * Remove circle `std::shared_ptr` dependencies
   */
  void invalidateSocket();

public: // WebSocket Listener methods

  CoroutineStarter onPing(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) override;
  CoroutineStarter onPong(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) override;
  CoroutineStarter onClose(const std::shared_ptr<AsyncWebSocket>& socket, v_uint16 code, const oatpp::String& message) override;
  CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket>& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;

private:
  void handleRequest(json request);
  void handleResponse(json response);
  void handleNotification(json notification);
private:
    std::map<int, nlohmann::json> m_sents;
};


#endif //ASYNC_SERVER_ROOMS_PEER_HPP
