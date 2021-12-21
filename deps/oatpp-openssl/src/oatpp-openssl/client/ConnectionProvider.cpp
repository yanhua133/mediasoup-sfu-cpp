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

#include "oatpp-openssl/Connection.hpp"

#include "oatpp/network/tcp/client/ConnectionProvider.hpp"

#include <openssl/crypto.h>

namespace oatpp { namespace openssl { namespace client {

ConnectionProvider::ConnectionProvider(const std::shared_ptr<Config>& config,
                                       const std::shared_ptr<oatpp::network::ClientConnectionProvider>& streamProvider)
  : m_config(config)
  , m_streamProvider(streamProvider)
  , m_ctx(nullptr)
{

  setProperty(PROPERTY_HOST, streamProvider->getProperty(PROPERTY_HOST).toString());
  setProperty(PROPERTY_PORT, streamProvider->getProperty(PROPERTY_PORT).toString());

  initSSLClient();

}

ConnectionProvider::~ConnectionProvider() {
  SSL_CTX_free(m_ctx);
}

void ConnectionProvider::stop() {
  m_streamProvider->stop();
}

void ConnectionProvider::initSSLClient() {

  auto method = SSLv23_client_method();

  m_ctx = SSL_CTX_new(method);
  if (!m_ctx) {
    throw std::runtime_error("[oatpp::openssl::client::ConnectionProvider::initSSLClient()]. Error. Can't create context.");
  }

  m_config->configureContext(m_ctx);

}

std::shared_ptr<ConnectionProvider> ConnectionProvider::createShared(const std::shared_ptr<Config>& config,
                                                                     const std::shared_ptr<oatpp::network::ClientConnectionProvider>& streamProvider) {
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
  
std::shared_ptr<data::stream::IOStream> ConnectionProvider::get(){

  oatpp::String host;
  auto hostName = m_streamProvider->getProperty(oatpp::network::ConnectionProvider::PROPERTY_HOST);
  if(hostName) {
    host = hostName.toString();
  }

  auto transportStream = m_streamProvider->get();

  if(!transportStream) {
    throw std::runtime_error("[oatpp::openssl::client::ConnectionProvider::get()]: Error. Can't connect.");
  }

  auto ssl = SSL_new(m_ctx);
  SSL_set_mode(ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);
  SSL_set_connect_state(ssl);
  SSL_set_tlsext_host_name(ssl, host->c_str());

  auto sslConnection = std::make_shared<Connection>(ssl, transportStream);

  sslConnection->setOutputStreamIOMode(data::stream::IOMode::BLOCKING);
  sslConnection->setInputStreamIOMode(data::stream::IOMode::BLOCKING);

  sslConnection->initContexts();

  return sslConnection;

}

oatpp::async::CoroutineStarterForResult<const std::shared_ptr<data::stream::IOStream>&> ConnectionProvider::getAsync() {


  class ConnectCoroutine : public oatpp::async::CoroutineWithResult<ConnectCoroutine, const std::shared_ptr<oatpp::data::stream::IOStream>&> {
  private:
    SSL_CTX* m_ctx;
    std::shared_ptr<Config> m_config;
    std::shared_ptr<oatpp::network::ClientConnectionProvider> m_streamProvider;
  private:
    std::shared_ptr<oatpp::data::stream::IOStream> m_stream;
    std::shared_ptr<Connection> m_connection;
  public:

    ConnectCoroutine(SSL_CTX* ctx, const std::shared_ptr<Config>& config, const std::shared_ptr<oatpp::network::ClientConnectionProvider>& streamProvider)
      : m_ctx(ctx)
      , m_config(config)
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

      oatpp::String host;
      auto hostName = m_streamProvider->getProperty(oatpp::network::ConnectionProvider::PROPERTY_HOST);
      if(hostName) {
        host = hostName.toString();
      }

      auto ssl = SSL_new(m_ctx);
      SSL_set_mode(ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);
      SSL_set_connect_state(ssl);
      SSL_set_tlsext_host_name(ssl, host->c_str());

      m_connection = std::make_shared<Connection>(ssl, m_stream);

      m_connection->setOutputStreamIOMode(oatpp::data::stream::IOMode::ASYNCHRONOUS);
      m_connection->setInputStreamIOMode(oatpp::data::stream::IOMode::ASYNCHRONOUS);

      return m_connection->initContextsAsync().next(yieldTo(&ConnectCoroutine::onSuccess));

    }

    Action onSuccess() {
      return _return(m_connection);
    }


  };

  return ConnectCoroutine::startForResult(m_ctx, m_config, m_streamProvider);

}

void ConnectionProvider::invalidate(const std::shared_ptr<data::stream::IOStream>& connection) {

  auto c = std::static_pointer_cast<oatpp::openssl::Connection>(connection);

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
