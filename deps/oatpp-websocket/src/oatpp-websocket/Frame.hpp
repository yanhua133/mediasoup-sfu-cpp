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

#ifndef oatpp_websocket_Frame_hpp
#define oatpp_websocket_Frame_hpp

#include "oatpp/core/Types.hpp"

namespace oatpp { namespace websocket {

/**
 * WebSocket frame.
 */
class Frame {
public:

  /**
   * Continuation frame.
   */
  static constexpr v_uint8 OPCODE_CONTINUATION = 0x0;

  /**
   * Text frame.
   */
  static constexpr v_uint8 OPCODE_TEXT = 0x1;

  /**
   * Binary frame.
   */
  static constexpr v_uint8 OPCODE_BINARY = 0x2;

  /**
   * Close frame.
   */
  static constexpr v_uint8 OPCODE_CLOSE = 0x8;

  /**
   * Ping frame.
   */
  static constexpr v_uint8 OPCODE_PING = 0x9;

  /**
   * Pong frame.
   */
  static constexpr v_uint8 OPCODE_PONG = 0xA;
  
public:

  /**
   * Frame header.
   */
  struct Header {
    /**
     * FIN bit.
     */
    bool fin;

    /**
     * RSV1 bit. Should be false.
     */
    bool rsv1;

    /**
     * RSV2 bit. Should be false.
     */
    bool rsv2;

    /**
     * RSV3 bit. Should be false.
     */
    bool rsv3;

    /**
     * Operation code.
     */
    v_uint8 opcode;

    /**
     * Mask bit. For client-to-server messages should be `true`. For server-to-client messages should be `false`.
     */
    bool hasMask;

    /**
     * Decoded payload length.
     */
    v_int64 payloadLength;

    /**
     * Payload mask.
     */
    v_uint8 mask[4] = {0, 0, 0, 0};
  };
  
public:

  /**
   * Serialize &l:Frame::Header;.
   * @param bits - out parameter. Two bytes header.
   * @param frameHeader - &l:Frame::Header;.
   * @param messageLengthScenario - out parameter. Depending on message length scenario. Message length can be encoded
   * in 1, 2, or 8 bytes.
   */
  static void packHeaderBits(v_uint16& bits, const Header& frameHeader, v_uint8& messageLengthScenario);

  /**
   * Deserialize &l:Frame::Header;.
   * @param bits - two bytes header.
   * @param frameHeader - out parameter. &l:Frame::Header;.
   * @param messageLen1 - first byte of encoded message length.
   */
  static void unpackHeaderBits(v_uint16 bits, Header& frameHeader, v_uint8& messageLen1);
  
};
  
}}

#endif /* oatpp_websocket_Frame_hpp */
