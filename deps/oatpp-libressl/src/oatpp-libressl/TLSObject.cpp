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

#include "TLSObject.hpp"

namespace oatpp { namespace libressl {

TLSObject::TLSObject(TLSHandle tlsHandle, Type type, const oatpp::String& serverName)
  : m_tlsHandle(tlsHandle)
  , m_type(type)
  , m_serverName(serverName)
  , m_closed(false)
{}

TLSObject::~TLSObject() {
  close();
}

TLSObject::TLSHandle TLSObject::getTLSHandle() {
  return m_tlsHandle;
}

TLSObject::Type TLSObject::getType() {
  return m_type;
}

oatpp::String TLSObject::getServerName() {
  return m_serverName;
}

void TLSObject::annul() {
  m_closed = true;
  m_tlsHandle = nullptr;
}

void TLSObject::close() {
  if(!m_closed) {
    m_closed = true;
    tls_close(m_tlsHandle);
    tls_free(m_tlsHandle);
  }
}

bool TLSObject::isClosed() {
  return m_closed;
}

}}
