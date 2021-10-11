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

#ifndef oatpp_websocket_AsyncWebSocket_hpp
#define oatpp_websocket_AsyncWebSocket_hpp

#include "./Frame.hpp"
#include "./Config.hpp"

#include "oatpp/core/data/stream/ChunkedBuffer.hpp"
#include "oatpp/core/async/Coroutine.hpp"

namespace oatpp { namespace websocket {

/**
 * Asynchronous WebSocket.
 */
class AsyncWebSocket : public oatpp::base::Countable, public std::enable_shared_from_this<AsyncWebSocket> {
public:
  /**
   * Convenience typedef for &id:oatpp::async::Action;.
   */
  typedef oatpp::async::Action Action;

  /**
   * Convenince typedef for &id:oatpp::async::CoroutineStarter;.
   */
  typedef oatpp::async::CoroutineStarter CoroutineStarter;
public:

  /**
   * Listener for websocket events.
   */
  class Listener {
  public:
    /**
     * Convenience typedef for &id:oatpp::async::Action;.
     */
    typedef oatpp::async::Action Action;

    /**
     * Convenince typedef for &id:oatpp::async::CoroutineStarter;.
     */
    typedef oatpp::async::CoroutineStarter CoroutineStarter;

    /**
     * Convenience typedef for &id:oatpp::websocket::AsyncWebSocket;.
     */
    typedef oatpp::websocket::AsyncWebSocket AsyncWebSocket;
  public:

    /**
     * Default virtual destructor.
     */
    virtual ~Listener() = default;

    /**
     * Called when "ping" frame received
     * @param socket - &id:oatpp::websocket::AsyncWebSocket;.
     * @param message - message received with the frame.
     * @return - &id:oatpp::async::CoroutineStarter;.
     * *To ignore this event return nullptr.*
     */
    virtual CoroutineStarter onPing(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) = 0;

    /**
     * Called when "pong" frame received
     * @param socket - &id:oatpp::websocket::AsyncWebSocket;.
     * @param message - message received with the frame.
     * @return - &id:oatpp::async::CoroutineStarter;. <br>
     * *To ignore this event return nullptr.*
     */
    virtual CoroutineStarter onPong(const std::shared_ptr<AsyncWebSocket>& socket, const oatpp::String& message) = 0;

    /**
     * Called when "close" frame received
     * @param socket - &id:oatpp::websocket::AsyncWebSocket;.
     * @param code - code of the websocket connection close message.
     * @param message - message text. &id:oatpp::String;.
     * @return - &id:oatpp::async::CoroutineStarter;. <br>
     * *To ignore this event return nullptr.*
     */
    virtual CoroutineStarter onClose(const std::shared_ptr<AsyncWebSocket>& socket, v_uint16 code, const oatpp::String& message) = 0;

    /**
     * Called when "text" or "binary" frame received. <br>
     * When all data of message is read, readMessage is called again with size == 0 to
     * indicate end of the message.
     * @param socket - &id:oatpp::websocket::AsyncWebSocket;.
     * @param opcode - &id:oatpp::websocket::Frame::OPCODE_TEXT; or &id:oatpp::websocket::Frame::OPCODE_BINARY;.
     * @param data - pointer to received data.
     * @param size - data size. &id:oatpp::v_io_size;.
     * @return - &id:oatpp::async::CoroutineStarter;. <br>
     * *To ignore this event return nullptr.*
     */
    virtual CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket>& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) = 0;
    
  };
  
private:
  
  bool checkForContinuation(const Frame::Header& frameHeader);

  CoroutineStarter readFrameHeaderAsync(const std::shared_ptr<Frame::Header>& frameHeader);
  
  /*
   * if(shortMessageStream == nullptr) - read call readMessage() method of listener
   * if(shortMessageStream) - read message to shortMessageStream. Don't call listener
   */
  CoroutineStarter readPayloadAsync(const std::shared_ptr<Frame::Header>& frameHeader,
                                    const std::shared_ptr<oatpp::data::stream::ChunkedBuffer>& shortMessageStream);

  CoroutineStarter handleFrameAsync(const std::shared_ptr<Frame::Header>& frameHeader);
  
private:
  Config m_config;
  std::shared_ptr<oatpp::data::stream::IOStream> m_connection;
  std::shared_ptr<Listener> m_listener;
  v_int32 m_lastOpcode;
  mutable bool m_listening;
public:

  /**
   * Constructor with Config.
   * @param connection - &id:oatpp::data::stream::IOStream;.
   * @param config - &id:oatpp::websocket::Config;.
   */
  AsyncWebSocket(const std::shared_ptr<oatpp::data::stream::IOStream>& connection, const Config& config);

  /**
   * Constructor.
   * @param connection - &id:oatpp::data::stream::IOStream;.
   * @param maskOutgoingMessages - for servers should be `false`. For clients should be `true`.
   */
  AsyncWebSocket(const std::shared_ptr<oatpp::data::stream::IOStream>& connection, bool maskOutgoingMessages);

  /**
   * Deleted copy-constructor.
   */
  AsyncWebSocket(const AsyncWebSocket&) = delete;
  AsyncWebSocket& operator=(const AsyncWebSocket&) = delete;
  
public:

  /**
   * Create shared AsyncWebSocket with Config.
   * @param connection - &id:oatpp::data::stream::IOStream;.
   * @param config - &id:oatpp::websocket::Config;.
   * @return - `std::shared_ptr` to AsyncWebSocket.
   */
  static std::shared_ptr<AsyncWebSocket> createShared(const std::shared_ptr<oatpp::data::stream::IOStream>& connection, const Config& config) {
    return std::make_shared<AsyncWebSocket>(connection, config);
  }

  /**
   * Create shared AsyncWebSocket.
   * @param connection - &id:oatpp::data::stream::IOStream;.
   * @param maskOutgoingMessages - for servers should be `false`. For clients should be `true`.
   * @return - `std::shared_ptr` to AsyncWebSocket.
   */
  static std::shared_ptr<AsyncWebSocket> createShared(const std::shared_ptr<oatpp::data::stream::IOStream>& connection, bool maskOutgoingMessages) {
    return std::make_shared<AsyncWebSocket>(connection, maskOutgoingMessages);
  }

  /**
   * Set WebSocket config.
   * @param config - &id:oatpp::websocket::Config;.
   */
  void setConfig(const Config& config);

  /**
   * Get socket connection.
   * @return - &id:oatpp::data::stream::IOStream;.
   */
  std::shared_ptr<oatpp::data::stream::IOStream> getConnection() const {
    return m_connection;
  }

  /**
   * Set socket event listener.
   * @param listener - &l:AsyncWebSocket::Listener;.
   */
  void setListener(const std::shared_ptr<Listener>& listener) {
    m_listener = listener;
  }

  /**
   * Get socket event listener
   * @return - &l:AsyncWebSocket::Listener;.
   */
  std::shared_ptr<Listener> getListener() {
    return m_listener;
  }

  /**
   * Start listening Coroutine. <br>
   * Read incoming frames and call corresponding methods of listener.
   * See &l:AsyncWebSocket::setListener ();.
   * @return - &id:oatpp::async::CoroutineStarter;.
   */
  CoroutineStarter listenAsync();

  /**
   * Send custom frame to peer.<br>
   * *Use this method if you know what you are doing.*
   * @param frameHeader - &id:oatpp::websocket::Frame::Header;.
   * @return - &id:oatpp::async::CoroutineStarter;.
   */
  CoroutineStarter writeFrameHeaderAsync(const std::shared_ptr<Frame::Header>& frameHeader);

  /**
   * Send default frame to peer with fin, opcode and messageSize set. <br>
   * *Use this method if you know what you are doing.*
   * @param frameHeader - &id:oatpp::websocket::Frame::Header;.
   * @param fin - FIN bit.
   * @param opcode - operation code. See &id:oatpp::websocket::Frame::Header;.
   * @param messageSize - size of the coming message.
   * @return - &id:oatpp::async::CoroutineStarter;.
   */
  CoroutineStarter sendFrameHeaderAsync(const std::shared_ptr<Frame::Header>& frameHeader, bool fin, v_uint8 opcode, v_int64 messageSize);

  /**
   * Send one frame message with custom fin and opcode.
   * @param fin - FIN bit.
   * @param opcode - operation code. See &id:oatpp::websocket::Frame::Header;.
   * @param message - message text. &id:oatpp::String;.
   * @return - &id:oatpp::async::CoroutineStarter;.
   */
  CoroutineStarter sendOneFrameAsync(bool fin, v_uint8 opcode, const oatpp::String& message);

  /**
   * Send close frame.
   * @param code - code of the websocket connection close message.
   * @param message - message text. &id:oatpp::String;.
   * @return - &id:oatpp::async::Action;.
   */
  CoroutineStarter sendCloseAsync(v_uint16 code, const oatpp::String& message);

  /**
   * Send close frame without message.
   * @return - &id:oatpp::async::CoroutineStarter;.
   */
  CoroutineStarter sendCloseAsync();

  /**
   * Send ping frame.
   * @param message - message text. &id:oatpp::String;.
   * @return - &id:oatpp::async::CoroutineStarter;.
   */
  CoroutineStarter sendPingAsync(const oatpp::String& message);

  /**
   * Send pong frame.
   * @param message - message text. &id:oatpp::String;.
   * @return - &id:oatpp::async::CoroutineStarter;.
   */
  CoroutineStarter sendPongAsync(const oatpp::String& message);

  /**
   * Send one frame text message.
   * @param message - message text. &id:oatpp::String;.
   * @return - &id:oatpp::async::CoroutineStarter;.
   */
  CoroutineStarter sendOneFrameTextAsync(const oatpp::String& message);

  /**
   * Send one frame binary message.
   * @param message - message text. &id:oatpp::String;.
   * @return - &id:oatpp::async::CoroutineStarter;.
   */
  CoroutineStarter sendOneFrameBinaryAsync(const oatpp::String& message);
  
};
  
}}

#endif /* oatpp_websocket_AsyncWebSocket_hpp */
