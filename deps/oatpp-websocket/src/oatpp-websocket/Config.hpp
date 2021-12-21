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

#ifndef oatpp_websocket_Config_hpp
#define oatpp_websocket_Config_hpp

#include "oatpp/core/Types.hpp"

namespace oatpp { namespace websocket {

/**
 * WebSocket configuration data.
 */
class Config {
public:

  /**
   * Mask outgoing messages payload. <br>
   * If `true` - Outgoing messages will be masked with random 32-bit masking key. <br>
   * For servers should be `false`. For clients should be `true`.
   */
  bool maskOutgoingMessages = false;

  /**
   * Size of the read buffer in bytes. <br>
   * Buffer used to read payload of the message. <br>
   * The smaller is buffer the more iterations will be made in order to read the whole message.
   */
  v_int32 readBufferSize = 1024;

};

}}

#endif // oatpp_websocket_Config_hpp
