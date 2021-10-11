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

#ifndef oatpp_websocket_ConnectionHandler_hpp
#define oatpp_websocket_ConnectionHandler_hpp

#include "WebSocket.hpp"

#include "oatpp/network/ConnectionHandler.hpp"

namespace oatpp { namespace websocket {

/**
 * Websocket connection handler. Extends &id:oatpp::network::ConnectionHandler;. <br>
 * Will create one thread per each connection to handle communication.
 */
class ConnectionHandler : public oatpp::base::Countable, public network::ConnectionHandler {
public:

  /**
   * Listener for new websocket instances.
   */
  class SocketInstanceListener {
  public:
    /**
     * Convenience typedef fo &id:oatpp::websocket::WebSocket;.
     */
    typedef oatpp::websocket::WebSocket WebSocket;

    /**
     * Convenience typedef for &id:oatpp::network::ConnectionHandler::ParameterMap;.
     */
    typedef oatpp::network::ConnectionHandler::ParameterMap ParameterMap;
  public:

    /**
     * Called when socket is created
     * @param socket - &id:oatpp::websocket::WebSocket;.
     * @param params - `std::shared_ptr` to const &id:oatpp::network::ConnectionHandler::ParameterMap;.
     */
    virtual void onAfterCreate(const WebSocket& socket, const std::shared_ptr<const ParameterMap>& params) = 0;

    /**
     * Called before socket instance is destroyed.
     * @param socket - &id:oatpp::websocket::WebSocket;.
     */
    virtual void onBeforeDestroy(const WebSocket& socket) = 0;
  };
private:
  std::shared_ptr<SocketInstanceListener> m_listener;
public:

  /**
   * Constructor.
   */
  ConnectionHandler();
public:

  /**
   * Create shared ConnectionHandler.
   * @return - `std::shared_ptr` to ConnectionHandler.
   */
  static std::shared_ptr<ConnectionHandler> createShared(){
    return std::make_shared<ConnectionHandler>();
  }

  /**
   * Set socket instance listener.
   * @param listener - &l:ConnectionHandler::SocketInstanceListener;.
   */
  void setSocketInstanceListener(const std::shared_ptr<SocketInstanceListener>& listener) {
    m_listener = listener;
  }

  /**
   * Implementation of &id:oatpp::network::ConnectionHandler::handleConnection;.
   * @param connection - &id:oatpp::data::stream::IOStream;.
   */
  void handleConnection(const std::shared_ptr<IOStream>& connection, const std::shared_ptr<const ParameterMap>& params) override;

  /**
   * Implementation of &id:oatpp::network::ConnectionHandler::stop;.
   * Here does nothing.
   */
  void stop() override {
    // DO NOTHING
  }
  
};
  
}}

#endif /* oatpp_websocket_ConnectionHandler_hpp */
