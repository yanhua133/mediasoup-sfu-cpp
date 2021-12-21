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

#include "Utils.hpp"
#include "oatpp/encoding/Base64.hpp"
#include "oatpp/core/utils/Random.hpp"

namespace oatpp { namespace websocket {

void Utils::generateMaskForFrame(Frame::Header& frameHeader) {
  oatpp::utils::random::Random::randomBytes(frameHeader.mask, 4);
}

oatpp::String Utils::generateKey() {
  oatpp::String key(16);
  oatpp::utils::random::Random::randomBytes(key->getData(), key->getSize());
  return oatpp::encoding::Base64::encode(key);
}

}}
