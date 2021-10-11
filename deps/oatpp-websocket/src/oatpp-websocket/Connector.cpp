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

#include "Connector.hpp"

#include "./Handshaker.hpp"

namespace oatpp { namespace websocket {

Connector::Connector(const std::shared_ptr<oatpp::network::ClientConnectionProvider>& connectionProvider)
  : m_connectionProvider(connectionProvider)
  , m_requestExecutor(connectionProvider)
{}

std::shared_ptr<Connector::Connection> Connector::connect(const oatpp::String& path, const Headers& headers) {

  auto connection = m_connectionProvider->get();
  if(!connection) {
    throw std::runtime_error("[oatpp::web::client::Connector::connectAndHandshake()]: Can't connect. Call to ConnectionProvider::get() failed.");
  }

  auto connectionProxy = std::make_shared<web::client::HttpRequestExecutor::ConnectionProxy>(m_connectionProvider, connection);
  auto connectionHandle = std::make_shared<web::client::HttpRequestExecutor::HttpConnectionHandle>(connectionProxy);
  
  Handshaker::Headers allHeaders;
  if(headers.getSize() > 0) {
    for(auto& h : headers.getAll()) {
      allHeaders.put(h.first, h.second);
    }
  }

  Handshaker::clientsideHandshake(allHeaders);
  auto response = m_requestExecutor.execute("GET", path, allHeaders, nullptr, connectionHandle);
  auto res = Handshaker::clientsideConfirmHandshake(allHeaders, response);

  if(res == Handshaker::STATUS_OK) {
    return connection;
  } else if(res == Handshaker::STATUS_SERVER_ERROR) {
    OATPP_LOGD("[oatpp::web::client::Connector::connectAndHandshake()]", "Server response code=%d", response->getStatusCode());
    throw std::runtime_error("[oatpp::web::client::Connector::connectAndHandshake()]: Server responded with invalid code");
  } else if(res == Handshaker::STATUS_SERVER_WRONG_KEY) {
    throw std::runtime_error("[oatpp::web::client::Connector::connectAndHandshake()]: Server wrong handshake key");
  } else if(res == Handshaker::STATUS_UNKNOWN_PROTOCOL_SUGGESTED) {
    throw std::runtime_error("[oatpp::web::client::Connector::connectAndHandshake()]: Server response contains unexpected headers");
  } else {
    throw std::runtime_error("[oatpp::web::client::Connector::connectAndHandshake()]: Unknown error");
  }
  
}

oatpp::async::CoroutineStarterForResult<const std::shared_ptr<Connector::Connection>&> Connector::connectAsync(const oatpp::String& path, const Headers& headers) {
  
  class ConnectCoroutine : public oatpp::async::CoroutineWithResult<ConnectCoroutine, const std::shared_ptr<Connection>&> {
  private:
    std::shared_ptr<oatpp::network::ClientConnectionProvider> m_connectionProvider;
    oatpp::web::client::HttpRequestExecutor m_requestExecutor;
    oatpp::String m_path;
    std::shared_ptr<Connection> m_connection;
    Handshaker::Headers m_handshakeHeaders;
  public:
    
    ConnectCoroutine(const std::shared_ptr<oatpp::network::ClientConnectionProvider>& connectionProvider,
                     const oatpp::web::client::HttpRequestExecutor& requestExecutor,
                     const oatpp::String path,
                     const Headers& headers)
      : m_connectionProvider(connectionProvider)
      , m_requestExecutor(requestExecutor)
      , m_path(path)
    {
      if(headers.getSize() > 0) {
        for(auto& h : headers.getAll()) {
          m_handshakeHeaders.put(h.first, h.second);
        }
      }
    }
    
    Action act() override {
      return m_connectionProvider->getAsync().callbackTo(&ConnectCoroutine::onConnected);
    }
    
    Action onConnected(const std::shared_ptr<Connection>& connection) {
      
      m_connection = connection;

      auto connectionProxy = std::make_shared<web::client::HttpRequestExecutor::ConnectionProxy>(m_connectionProvider, connection);
      auto connectionHandle = std::make_shared<web::client::HttpRequestExecutor::HttpConnectionHandle>(connectionProxy);
      Handshaker::clientsideHandshake(m_handshakeHeaders);

      return m_requestExecutor.executeAsync("GET", m_path, m_handshakeHeaders, nullptr, connectionHandle).callbackTo(&ConnectCoroutine::onServerResponse);
      
    }
    
    Action onServerResponse(const std::shared_ptr<oatpp::web::protocol::http::incoming::Response>& response) {
      auto res = Handshaker::clientsideConfirmHandshake(m_handshakeHeaders, response);
      if(res == Handshaker::STATUS_OK) {
        return _return(m_connection);
      } else if(res == Handshaker::STATUS_SERVER_ERROR) {
        throw std::runtime_error("[oatpp::web::client::Connector::connectAsync(){ConnectCoroutine}]: Server responded with invalid code");
      } else if(res == Handshaker::STATUS_SERVER_WRONG_KEY) {
        throw std::runtime_error("[oatpp::web::client::Connector::connectAsync(){ConnectCoroutine}]: Server wrong handshake key");
      } else if(res == Handshaker::STATUS_UNKNOWN_PROTOCOL_SUGGESTED) {
        throw std::runtime_error("[oatpp::web::client::Connector::connectAsync(){ConnectCoroutine}]: Server response contains unexpected headers");
      } else {
        throw std::runtime_error("[oatpp::web::client::Connector::connectAsync(){ConnectCoroutine}]: Unknown error");
      }
    }
    
  };
  
  return ConnectCoroutine::startForResult(m_connectionProvider, m_requestExecutor, path, headers);
  
}
  
}}
