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

#include "oatpp/network/tcp/client/ConnectionProvider.hpp"

#include <openssl/crypto.h>

namespace oatpp { namespace libressl { namespace client {

ConnectionProvider::ConnectionProvider(const std::shared_ptr<Config>& config,
                                       const std::shared_ptr<oatpp::network::ClientConnectionProvider>& streamProvider)
  : m_config(config)
  , m_streamProvider(streamProvider)
{

  setProperty(PROPERTY_HOST, streamProvider->getProperty(PROPERTY_HOST).toString());
  setProperty(PROPERTY_PORT, streamProvider->getProperty(PROPERTY_PORT).toString());

  auto calback = CRYPTO_get_locking_callback();
  if(!calback) {
    OATPP_LOGD("[oatpp::libressl::client::ConnectionProvider::ConnectionProvider()]",
               "WARNING. libressl. CRYPTO_set_locking_callback is NOT set. "
               "This can cause problems using libressl in multithreaded environment! "
               "Please call oatpp::libressl::Callbacks::setDefaultCallbacks() or "
               "consider setting custom locking_callback.");
  }

}

std::shared_ptr<ConnectionProvider> ConnectionProvider::createShared(const std::shared_ptr<Config>& config,
                                                                     const std::shared_ptr<oatpp::network::ClientConnectionProvider>& streamProvider)
{
  return std::shared_ptr<ConnectionProvider>(new ConnectionProvider(config, streamProvider));
}

std::shared_ptr<ConnectionProvider> ConnectionProvider::createShared(const std::shared_ptr<Config>& config,
                                                                     const network::Address& address)
{
  return createShared(
    config,
    network::tcp::client::ConnectionProvider::createShared(address)
  );
}

  
std::shared_ptr<data::stream::IOStream> ConnectionProvider::get() {

  Connection::TLSHandle tlsHandle = tls_client();
  tls_configure(tlsHandle, m_config->getTLSConfig());

  oatpp::String host;
  auto hostName = m_streamProvider->getProperty(oatpp::network::ConnectionProvider::PROPERTY_HOST);
  if(hostName) {
    host = hostName.toString();
  }

  auto tlsObject = std::make_shared<TLSObject>(tlsHandle, TLSObject::Type::CLIENT, host);
  auto connection = std::make_shared<Connection>(tlsObject, m_streamProvider->get());

  connection->setOutputStreamIOMode(oatpp::data::stream::IOMode::BLOCKING);
  connection->setInputStreamIOMode(oatpp::data::stream::IOMode::BLOCKING);

  connection->initContexts();
  return connection;

}

oatpp::async::CoroutineStarterForResult<const std::shared_ptr<data::stream::IOStream>&> ConnectionProvider::getAsync() {


  class ConnectCoroutine : public oatpp::async::CoroutineWithResult<ConnectCoroutine, const std::shared_ptr<oatpp::data::stream::IOStream>&> {
  private:
    std::shared_ptr<Config> m_config;
    std::shared_ptr<oatpp::network::ClientConnectionProvider> m_streamProvider;
  private:
    std::shared_ptr<oatpp::data::stream::IOStream> m_stream;
    std::shared_ptr<Connection> m_connection;
  public:

    ConnectCoroutine(const std::shared_ptr<Config>& config, const std::shared_ptr<oatpp::network::ClientConnectionProvider>& streamProvider)
      : m_config(config)
      , m_streamProvider(streamProvider)
    {}

    Action act() override {
      /* get transport stream */
      return m_streamProvider->getAsync().callbackTo(&ConnectCoroutine::onConnected);
    }

    Action onConnected(const std::shared_ptr<oatpp::data::stream::IOStream>& stream) {
      /* transport stream obtained */
      m_stream = stream;
      return yieldTo(&ConnectCoroutine::secureConnection);
    }

    Action secureConnection() {

      Connection::TLSHandle tlsHandle = tls_client();
      tls_configure(tlsHandle, m_config->getTLSConfig());

      oatpp::String host;
      auto hostName = m_streamProvider->getProperty(oatpp::network::ConnectionProvider::PROPERTY_HOST);
      if(hostName) {
        host = hostName.toString();
      }

      auto tlsObject = std::make_shared<TLSObject>(tlsHandle, TLSObject::Type::CLIENT, host);
      m_connection = std::make_shared<Connection>(tlsObject, m_stream);

      m_connection->setOutputStreamIOMode(oatpp::data::stream::IOMode::ASYNCHRONOUS);
      m_connection->setInputStreamIOMode(oatpp::data::stream::IOMode::ASYNCHRONOUS);

      return m_connection->initContextsAsync().next(yieldTo(&ConnectCoroutine::onSuccess));

    }

    Action onSuccess() {
      return _return(m_connection);
    }


  };

  return ConnectCoroutine::startForResult(m_config, m_streamProvider);

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
