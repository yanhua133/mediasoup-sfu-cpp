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

#ifndef ASYNC_SERVER_ROOMS_LOBBY_HPP
#define ASYNC_SERVER_ROOMS_LOBBY_HPP

#include "./Room.hpp"
#include "../utils/Statistics.hpp"

#include "oatpp-websocket/AsyncConnectionHandler.hpp"

#include <unordered_map>
#include <mutex>

class Lobby : public oatpp::websocket::AsyncConnectionHandler::SocketInstanceListener {
public:
  std::atomic<v_int64> m_peerIdCounter;
  std::unordered_map<oatpp::String, std::shared_ptr<Room>> m_rooms;
  std::mutex m_roomsMutex;
private:
  OATPP_COMPONENT(std::shared_ptr<Statistics>, m_statistics);
public:

  Lobby()
    : m_peerIdCounter(1)
  {}

  /**
   * Generate id for new user
   * @return
   */
  std::string obtainNewPeerId();

  /**
   * Get room by name or create new one if not exists.
   * @param roomName
   * @return
   */
  std::shared_ptr<Room> getOrCreateRoom(const oatpp::String& roomName);

  /**
   * Get room by name.
   * @param roomName
   * @return
   */
  std::shared_ptr<Room> getRoom(const oatpp::String& roomName);

  /**
   * Delete room by name.
   * @param roomName
   */
  void deleteRoom(const oatpp::String& roomName);

  /**
   * Websocket-Ping all peers in the loop. Each time `interval`.
   * @param interval
   */
  void runPingLoop(const std::chrono::duration<v_int64, std::micro>& interval = std::chrono::minutes(1));

public:

  /**
   *  Called when socket is created
   */
  void onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket, const std::shared_ptr<const ParameterMap>& params) override;

  /**
   *  Called before socket instance is destroyed.
   */
  void onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket) override;

};


#endif //ASYNC_SERVER_ROOMS_LOBBY_HPP
