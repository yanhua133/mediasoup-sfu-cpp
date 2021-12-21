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

#ifndef oatpp_libressl_Callbacks_hpp
#define oatpp_libressl_Callbacks_hpp

#include "oatpp/core/Types.hpp"

namespace oatpp { namespace libressl {

/**
 * Collection of default-implemented callbacks for
 * libressl
 */
class Callbacks {
private:
  /*
   * Atomics for lockingCallback;
   */
  static oatpp::concurrency::SpinLock* LOCKS;
private:
  /*
   * Init atomics for lockingCallback;
   */
  static oatpp::concurrency::SpinLock* createLocks();
public:
  
  /**
   * Set default callbacks for libressl
   */
  static void setDefaultCallbacks();

  /**
   * Oatpp-default implementation of lockingCallback passed to CRYPTO_set_locking_callback().
   * must be set in case libressl is used in multithreaded environment.
   * Locking is done using &id:oatpp::concurrency::SpinLock;.
   * @param mode
   * @param n - index of the lock.
   * @param file - file where lock is set.
   * @param line - line where lock is set.
   */
  static void lockingCallback(int mode, int n, const char* file, int line);
  
};
  
}}

#endif /* oatpp_libressl_Callbacks_hpp */
