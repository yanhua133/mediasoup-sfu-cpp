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

#include "Callbacks.hpp"

#include <openssl/crypto.h>

namespace oatpp { namespace libressl {

oatpp::concurrency::SpinLock* Callbacks::LOCKS = Callbacks::createLocks();
  
void Callbacks::setDefaultCallbacks() {
  CRYPTO_set_locking_callback(Callbacks::lockingCallback);
}
  
oatpp::concurrency::SpinLock* Callbacks::createLocks() {
  return new oatpp::concurrency::SpinLock[CRYPTO_num_locks()];
}
  
void Callbacks::lockingCallback(int mode, int n, const char* file, int line) {
  if (mode & CRYPTO_LOCK) {
    LOCKS[n].lock();
  } else {
    LOCKS[n].unlock();
  }
}
  
}}
