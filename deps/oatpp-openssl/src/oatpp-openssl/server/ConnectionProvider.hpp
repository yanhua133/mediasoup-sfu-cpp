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

#ifndef oatpp_openssl_server_ConnectionProvider_hpp
#define oatpp_openssl_server_ConnectionProvider_hpp

#include "oatpp-openssl/Config.hpp"

#include "oatpp/network/Address.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"

namespace oatpp { namespace openssl { namespace server {

/**
 * Openssl server connection provider.
 * Extends &id:oatpp::base::Countable;, &id:oatpp::network::ServerConnectionProvider;.
 */
class ConnectionProvider : public oatpp::base::Countable, public oatpp::network::ServerConnectionProvider {
private:
  std::shared_ptr<Config> m_config;
  std::shared_ptr<oatpp::network::ServerConnectionProvider> m_streamProvider;
private:
  void instantiateTLSServer();
private:
  SSL_CTX* m_ctx;
public:
  /**
   * Constructor.
   * @param config - &id:oatpp::openssl::Config;.
   * @param streamProvider - provider of underlying transport stream. &id:oatpp::network::ServerConnectionProvider;.
   */
  ConnectionProvider(const std::shared_ptr<Config>& config,
                     const std::shared_ptr<oatpp::network::ServerConnectionProvider>& streamProvider);
public:

  /**
   * Create shared ConnectionProvider.
   * @param config - &id:oatpp::openssl::Config;.
   * @param streamProvider - provider of underlying transport stream. &id:oatpp::network::ServerConnectionProvider;.
   * @return - `std::shared_ptr` to ConnectionProvider.
   */
  static std::shared_ptr<ConnectionProvider> createShared(const std::shared_ptr<Config>& config,
                                                          const std::shared_ptr<oatpp::network::ServerConnectionProvider>& streamProvider);

  /**
   * Create shared ConnectionProvider.
   * @param config - &id:oatpp::openssl::Config;.
   * @param address - &id:oatpp::network::Address;.
   * @param useExtendedConnections - set `true` to use &l:ConnectionProvider::ExtendedConnection;.
   * `false` to use &id:oatpp::network::tcp::Connection;.
   * @return - `std::shared_ptr` to ConnectionProvider.
   */
  static std::shared_ptr<ConnectionProvider> createShared(const std::shared_ptr<Config>& config,
                                                          const network::Address& address,
                                                          bool useExtendedConnections = false);


  /**
   * Virtual destructor.
   */
  ~ConnectionProvider();

  /**
   * Close all handles.
   */
  void stop() override;

  /**
   * Get incoming connection.
   * @return &id:oatpp::data::stream::IOStream;.
   */
  std::shared_ptr<data::stream::IOStream> get() override;

  /**
   * No need to implement this.<br>
   * For Asynchronous IO in oatpp it is considered to be a good practice
   * to accept connections in a seperate thread with the blocking accept()
   * and then process connections in Asynchronous manner with non-blocking read/write.
   * <br>
   * *It may be implemented later*
   */
  oatpp::async::CoroutineStarterForResult<const std::shared_ptr<data::stream::IOStream>&> getAsync() override {
    /*
     *  No need to implement this.
     *  For Asynchronous IO in oatpp it is considered to be a good practice
     *  to accept connections in a seperate thread with the blocking accept()
     *  and then process connections in Asynchronous manner with non-blocking read/write
     */
    throw std::runtime_error("oatpp::openssl::server::ConnectionProvider::getAsync not implemented.");
  }

  /**
   * Will call `invalidateConnection()` for the underlying transport stream.
   * @param connection - **MUST** be an instance of &id:oatpp::openssl::Connection;.
   */
  void invalidate(const std::shared_ptr<data::stream::IOStream>& connection) override;
  
};
  
}}}

#endif /* oatpp_openssl_server_ConnectionProvider_hpp */
