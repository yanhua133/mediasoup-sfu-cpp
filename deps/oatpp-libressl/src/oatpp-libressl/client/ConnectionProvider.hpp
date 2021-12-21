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

#ifndef oatpp_libressl_client_ConnectionProvider_hpp
#define oatpp_libressl_client_ConnectionProvider_hpp

#include "oatpp-libressl/Config.hpp"
#include "oatpp-libressl/TLSObject.hpp"

#include "oatpp/network/Address.hpp"
#include "oatpp/network/ConnectionProvider.hpp"

namespace oatpp { namespace libressl { namespace client {

/**
 * Libressl client connection provider.
 * Extends &id:oatpp::base::Countable;, &id:oatpp::network::ClientConnectionProvider;.
 */
class ConnectionProvider : public base::Countable, public oatpp::network::ClientConnectionProvider {
private:
  std::shared_ptr<Config> m_config;
  std::shared_ptr<oatpp::network::ClientConnectionProvider> m_streamProvider;
  bool m_closed;
  std::shared_ptr<TLSObject> m_tlsObject;
public:
  /**
   * Constructor.
   * @param config - &id:oatpp::libressl::Config;.
   * @param streamProvider - provider of underlying transport stream. &id:oatpp::network::ClientConnectionProvider;.
   */
  ConnectionProvider(const std::shared_ptr<Config>& config, const std::shared_ptr<oatpp::network::ClientConnectionProvider>& streamProvider);
public:

  /**
   * Create shared ConnectionProvider.
   * @param config - &id:oatpp::libressl::Config;.
   * @param streamProvider - provider of underlying transport stream. &id:oatpp::network::ClientConnectionProvider;.
   * @return - `std::shared_ptr` to ConnectionProvider.
   */
  static std::shared_ptr<ConnectionProvider> createShared(const std::shared_ptr<Config>& config,
                                                          const std::shared_ptr<oatpp::network::ClientConnectionProvider>& streamProvider);

  /**
   * Create shared ConnectionProvider.
   * @param config - &id:oatpp::libressl::Config;.
   * @param address - &id:oatpp::network::Address;.
   * @param useExtendedConnections - set `true` to use &l:ConnectionProvider::ExtendedConnection;.
   * `false` to use &id:oatpp::network::tcp::Connection;.
   * @return - `std::shared_ptr` to ConnectionProvider.
   */
  static std::shared_ptr<ConnectionProvider> createShared(const std::shared_ptr<Config>& config,
                                                          const network::Address& address);

  /**
   * Implements &id:oatpp::network::ConnectionProvider::close;. Here does nothing.
   */
  void stop() override {
    // DO NOTHING
  }

  /**
   * Get connection.
   * @return - `std::shared_ptr` to &id:oatpp::data::stream::IOStream;.
   */
  std::shared_ptr<data::stream::IOStream> get() override;

  /**
   * Get connection in asynchronous manner.
   * @return - &id:oatpp::async::CoroutineStarterForResult;.
   */
  oatpp::async::CoroutineStarterForResult<const std::shared_ptr<oatpp::data::stream::IOStream>&> getAsync() override;

  /**
   * Will call `invalidateConnection()` for the underlying transport stream.
   * @param connection - **MUST** be an instance of &id:oatpp::libressl::Connection;.
   */
  void invalidate(const std::shared_ptr<data::stream::IOStream>& connection) override;

};
  
}}}

#endif /* oatpp_libressl_client_ConnectionProvider_hpp */
