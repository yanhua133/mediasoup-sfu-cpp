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

#ifndef DTOs_hpp
#define DTOs_hpp

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(MessageCodes, v_int32,
  VALUE(CODE_INFO, 0),

  VALUE(CODE_PEER_JOINED, 1),
  VALUE(CODE_PEER_LEFT, 2),
  VALUE(CODE_PEER_MESSAGE, 3),
  VALUE(CODE_PEER_MESSAGE_FILE, 4),
  VALUE(CODE_PEER_IS_TYPING, 5),

  VALUE(CODE_FILE_SHARE, 6),
  VALUE(CODE_FILE_REQUEST_CHUNK, 7),
  VALUE(CODE_FILE_CHUNK_DATA, 8),

  VALUE(CODE_API_ERROR, 9)
);

class PeerDto : public oatpp::DTO {
public:

  DTO_INIT(PeerDto, DTO)

  DTO_FIELD(String, peerId);
  DTO_FIELD(String, peerName);

};

class MessageDto : public oatpp::DTO {
public:

  DTO_INIT(MessageDto, DTO)

  DTO_FIELD(String, peerId);
  DTO_FIELD(String, peerName);
  DTO_FIELD(Enum<MessageCodes>::AsNumber::NotNull, code);
  DTO_FIELD(String, message);
  DTO_FIELD(Int64, timestamp);

  DTO_FIELD(List<Object<PeerDto>>, peers);
  DTO_FIELD(List<Object<MessageDto>>, history);

};

class StatPointDto : public oatpp::DTO {

  DTO_INIT(StatPointDto, DTO);

  DTO_FIELD(Int64, timestamp);

  DTO_FIELD(UInt64, evFrontpageLoaded, "ev_front_page_loaded");

  DTO_FIELD(UInt64, evPeerConnected, "ev_peer_connected");
  DTO_FIELD(UInt64, evPeerDisconnected, "ev_peer_disconnected");
  DTO_FIELD(UInt64, evPeerZombieDropped, "ev_peer_zombie_dropped");
  DTO_FIELD(UInt64, evPeerSendMessage, "ev_peer_send_message");
  DTO_FIELD(UInt64, evPeerShareFile, "ev_peer_share_file");

  DTO_FIELD(UInt64, evRoomCreated, "ev_room_created");
  DTO_FIELD(UInt64, evRoomDeleted, "ev_room_deleted");

  DTO_FIELD(UInt64, fileServedBytes, "file_served_bytes");

};

#include OATPP_CODEGEN_END(DTO)

#endif // DTOs_hpp
