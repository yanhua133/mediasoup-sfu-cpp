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

#include "ConnectionHandler.hpp"
#include "oatpp/core/concurrency/Thread.hpp"

namespace oatpp { namespace websocket {

ConnectionHandler::ConnectionHandler()
  : m_listener(nullptr)
{}

void ConnectionHandler::handleConnection(const std::shared_ptr<IOStream>& connection, const std::shared_ptr<const ParameterMap>& params) {
  
  class Task : public base::Countable {
    std::shared_ptr<IOStream> m_connection;
    std::shared_ptr<const ParameterMap> m_params;
    std::shared_ptr<SocketInstanceListener> m_listener;
    std::shared_ptr<WebSocket> m_socket;
  public:

    Task(const Task&) = delete;

    Task(const std::shared_ptr<IOStream>& connection,
         const std::shared_ptr<const ParameterMap>& params,
         const std::shared_ptr<SocketInstanceListener>& listener)
      : m_connection(connection)
      , m_params(params)
      , m_listener(listener)
      , m_socket(std::make_shared<WebSocket>(connection, false))
    {
      if(m_listener) {
        m_listener->onAfterCreate(*m_socket, params);
      }
    }

    Task(Task&& other)
      : m_connection(std::move(other.m_connection))
      , m_listener(std::move(other.m_listener))
      , m_socket(std::move(other.m_socket))
    {}
    
    ~Task() {
      if(m_listener){
        m_listener->onBeforeDestroy(*m_socket);
      }
    }
    
    void run() {
      m_socket->listen();
    }

    Task& operator = (Task&& other){
      m_connection = std::move(other.m_connection);
      m_listener = std::move(other.m_listener);
      m_socket = std::move(other.m_socket);
      return *this;
    }
    
  };

  connection->setInputStreamIOMode(oatpp::data::stream::IOMode::BLOCKING);
  connection->setOutputStreamIOMode(oatpp::data::stream::IOMode::BLOCKING);
  
  /* Create working thread */
  std::thread thread(&Task::run, Task(connection, params, m_listener));
  
  /* Get hardware concurrency -1 in order to have 1cpu free of workers. */
  v_int32 concurrency = oatpp::concurrency::getHardwareConcurrency();
  if(concurrency > 1) {
    concurrency -= 1;
  }
  
  /* Set thread affinity group CPUs [0..cpu_count - 1]. Leave one cpu free of workers */
  oatpp::concurrency::setThreadAffinityToCpuRange(thread.native_handle(), 0, concurrency - 1 /* -1 because 0-based index */);
  
  thread.detach();
  
}
  
}}
