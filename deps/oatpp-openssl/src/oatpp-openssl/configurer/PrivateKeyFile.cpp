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

#include "PrivateKeyFile.hpp"

namespace oatpp { namespace openssl { namespace configurer {

PrivateKeyFile::PrivateKeyFile(const oatpp::String &filename, int filetype)
  : m_filename(filename)
  , m_filetype(filetype)
{}

void PrivateKeyFile::configure(SSL_CTX *ctx) {
  if (SSL_CTX_use_PrivateKey_file(ctx, m_filename->c_str(), m_filetype) <= 0) {
    throw std::runtime_error("[oatpp::openssl::configurer::PrivateKeyFile::configure()]: Error. "
                             "Call to 'SSL_CTX_use_PrivateKey_file' failed.");
  }
}

}}}

