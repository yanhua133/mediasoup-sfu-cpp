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

#include "ConnectionProvider.hpp"

#include "oatpp-libressl/Connection.hpp"

#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/core/utils/ConversionUtils.hpp"

namespace oatpp { namespace libressl { namespace server {

ConnectionProvider::ConnectionProvider(const std::shared_ptr<Config>& config,
                                       const std::shared_ptr<oatpp::network::ServerConnectionProvider>& streamProvider)
  : m_config(config)
  , m_streamProvider(streamProvider)
  , m_closed(false)
{

  setProperty(PROPERTY_HOST, streamProvider->getProperty(PROPERTY_HOST).toString());
  setProperty(PROPERTY_PORT, streamProvider->getProperty(PROPERTY_PORT).toString());

  m_tlsObject = instantiateTLSServer();

}

std::shared_ptr<ConnectionProvider> ConnectionProvider::createShared(const std::shared_ptr<Config>& config,
                                                                     const std::shared_ptr<oatpp::network::ServerConnectionProvider>& streamProvider)
{
  return std::shared_ptr<ConnectionProvider>(new ConnectionProvider(config, streamProvider));
}

std::shared_ptr<ConnectionProvider> ConnectionProvider::createShared(const std::shared_ptr<Config>& config,
                                                                     const network::Address& address,
                                                                     bool useExtendedConnections)
{
  return createShared(
    config,
    network::tcp::server::ConnectionProvider::createShared(address, useExtendedConnections)
  );
}

ConnectionProvider::~ConnectionProvider() {
  stop();
}

std::shared_ptr<TLSObject> ConnectionProvider::instantiateTLSServer() {

  Connection::TLSHandle handle = tls_server();

  if(handle == NULL) {
    throw std::runtime_error("[oatpp::libressl::server::ConnectionProvider::instantiateTLSServer()]: Failed to create tls_server");
  }

  if (tls_configure(handle, m_config->getTLSConfig()) < 0) {
    OATPP_LOGD("[oatpp::libressl::server::ConnectionProvider::instantiateTLSServer()]", "Error on call to 'tls_configure'. %s", tls_error(handle));
    throw std::runtime_error( "[oatpp::libressl::server::ConnectionProvider::instantiateTLSServer()]: Failed to configure tls_server");
  }

  return std::make_shared<TLSObject>(handle, TLSObject::Type::SERVER, nullptr);

}

void ConnectionProvider::stop() {
  if(!m_closed) {
    m_closed = true;
    if(m_tlsObject) {
      m_tlsObject->close();
    }
    m_streamProvider->stop();
  }
}

std::shared_ptr<data::stream::IOStream> ConnectionProvider::get(){
  auto transportStream = m_streamProvider->get();
  if(transportStream) {
    return std::make_shared<Connection>(m_tlsObject, transportStream);
  }
  return nullptr;
}

void ConnectionProvider::invalidate(const std::shared_ptr<data::stream::IOStream>& connection) {

  auto c = std::static_pointer_cast<oatpp::libressl::Connection>(connection);

  /********************************************
   * WARNING!!!
   *
   * c->closeTLS(); <--- DO NOT
   *
   * DO NOT CLOSE or DELETE TLS handles here.
   * Remember - other threads can still be
   * waiting for TLS events.
   ********************************************/

  /* Invalidate underlying transport */
  auto s = c->getTransportStream();
  m_streamProvider->invalidate(s);

}

}}}

