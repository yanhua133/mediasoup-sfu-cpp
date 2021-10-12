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

#ifndef Statistics_hpp
#define Statistics_hpp

#include "../dto/DTOs.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/Types.hpp"

#include <chrono>

class Statistics {
public:

  std::atomic<v_uint64> EVENT_FRONT_PAGE_LOADED   {0};          // On Frontpage Loaded

  std::atomic<v_uint64> EVENT_PEER_CONNECTED      {0};          // On Connected event counter
  std::atomic<v_uint64> EVENT_PEER_DISCONNECTED   {0};          // On Disconnected event counter
  std::atomic<v_uint64> EVENT_PEER_ZOMBIE_DROPPED {0};          // On Disconnected due to failed ping counter
  std::atomic<v_uint64> EVENT_PEER_SEND_MESSAGE   {0};          // Sent messages counter
  std::atomic<v_uint64> EVENT_PEER_SHARE_FILE     {0};          // Shared files counter

  std::atomic<v_uint64> EVENT_ROOM_CREATED        {0};          // On room created
  std::atomic<v_uint64> EVENT_ROOM_DELETED        {0};          // On room deleted

  std::atomic<v_uint64> FILE_SERVED_BYTES         {0};          // Overall shared files served bytes

private:
  oatpp::parser::json::mapping::ObjectMapper m_objectMapper;
private:
  oatpp::List<oatpp::Object<StatPointDto>> m_dataPoints = oatpp::List<oatpp::Object<StatPointDto>>::createShared();
  std::mutex m_dataLock;
private:
  std::chrono::duration<v_int64, std::micro> m_maxPeriod;
  std::chrono::duration<v_int64, std::micro> m_pushInterval;
  std::chrono::duration<v_int64, std::micro> m_updateInterval;
public:

  Statistics(const std::chrono::duration<v_int64, std::micro>& maxPeriod = std::chrono::hours(7 * 24),
             const std::chrono::duration<v_int64, std::micro>& pushInterval = std::chrono::hours(1),
             const std::chrono::duration<v_int64, std::micro>& updateInterval = std::chrono::seconds(1))
    : m_maxPeriod(maxPeriod)
    , m_pushInterval(pushInterval)
    , m_updateInterval(updateInterval)
  {}

  void takeSample();
  oatpp::String getJsonData();

  void runStatLoop();

};

#endif // Statistics_hpp
