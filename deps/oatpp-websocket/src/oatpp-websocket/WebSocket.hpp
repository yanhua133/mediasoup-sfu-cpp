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

#ifndef oatpp_websocket_WebSocket_hpp
#define oatpp_websocket_WebSocket_hpp

#include "./Frame.hpp"
#include "./Config.hpp"

#include "oatpp/core/data/stream/ChunkedBuffer.hpp"

namespace oatpp { namespace websocket {

/**
 * WebSocket.
 */
class WebSocket : public oatpp::base::Countable {
public:

  /**
   * Listener for websocket events.
   */
  class Listener {
  public:
    /**
     * Convenience typedef fo &id:oatpp::websocket::WebSocket;.
     */
    typedef oatpp::websocket::WebSocket WebSocket;
  public:

    /**
     * Default virtual destructor.
     */
    virtual ~Listener() = default;

    /**
     * Called when "ping" frame received
     * @param socket - &id:oatpp::websocket::WebSocket;.
     * @param message - message text; &id:oatpp::String;.
     */
    virtual void onPing(const WebSocket& socket, const oatpp::String& message) = 0;

    /**
     * Called when "pong" frame received
     * @param socket - &id:oatpp::websocket::WebSocket;.
     * @param message - message text; &id:oatpp::String;.
     */
    virtual void onPong(const WebSocket& socket, const oatpp::String& message) = 0;

    /**
     * Called when "close" frame received
     * @param socket - &id:oatpp::websocket::WebSocket;.
     * @param code - close frame message code.
     * @param message - message text; &id:oatpp::String;.
     */
    virtual void onClose(const WebSocket& socket, v_uint16 code, const oatpp::String& message) = 0;

    /**
     * Called when "text" or "binary" frame received. <br>
     * When all data of message is read, readMessage is called again with size == 0 to
     * indicate end of the message.
     * @param socket - &id:oatpp::websocket::WebSocket;.
     * @param opcode - &id:oatpp::websocket::Frame::OPCODE_TEXT; or &id:oatpp::websocket::Frame::OPCODE_BINARY;.
     * @param data - pointer to message data.
     * @param size - data size.
     */
    virtual void readMessage(const WebSocket& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) = 0;
    
  };
  
private:
  
  bool checkForContinuation(const Frame::Header& frameHeader);
  void readFrameHeader(Frame::Header& frameHeader) const;
  
  /**
   * if(shortMessageStream == nullptr) - read call readMessage() method of listener
   * if(shortMessageStream) - read message to shortMessageStream. Don't call listener
   */
  void readPayload(const Frame::Header& frameHeader, oatpp::data::stream::ChunkedBuffer* shortMessageStream) const;
  
  void handleFrame(const Frame::Header& frameHeader);

private:
  Config m_config;
  std::shared_ptr<oatpp::data::stream::IOStream> m_connection;
  mutable std::shared_ptr<Listener> m_listener;
  v_int32 m_lastOpcode;
  mutable bool m_listening;
public:


  /**
   * Constructor with Config.
   * @param connection - &id:oatpp::data::stream::IOStream;.
   * @param config - &id:oatpp::websocket::Config;.
   */
  WebSocket(const std::shared_ptr<oatpp::data::stream::IOStream>& connection, const Config& config);

  /**
   * Constructor.
   * @param connection - &id:oatpp::data::stream::IOStream;.
   * @param maskOutgoingMessages - for servers should be `false`. For clients should be `true`.
   */
  WebSocket(const std::shared_ptr<oatpp::data::stream::IOStream>& connection, bool maskOutgoingMessages);

  /**
   * Deleted copy-constructor.
   */
  WebSocket(const WebSocket&) = delete;
  WebSocket& operator=(const WebSocket&) = delete;
  
public:

  /**
   * Create shared WebSocket with Config.
   * @param connection - &id:oatpp::data::stream::IOStream;.
   * @param config - &id:oatpp::websocket::Config;.
   * @return - `std::shared_ptr` to WebSocket.
   */
  static std::shared_ptr<WebSocket> createShared(const std::shared_ptr<oatpp::data::stream::IOStream>& connection, const Config& config) {
    return std::make_shared<WebSocket>(connection, config);
  }

  /**
   * Create shared WebSocket.
   * @param connection - &id:oatpp::data::stream::IOStream;.
   * @param maskOutgoingMessages - for servers should be `false`. For clients should be `true`.
   * @return
   */
  static std::shared_ptr<WebSocket> createShared(const std::shared_ptr<oatpp::data::stream::IOStream>& connection, bool maskOutgoingMessages) {
    return std::make_shared<WebSocket>(connection, maskOutgoingMessages);
  }

  /**
   * Set WebSocket config.
   * @param config - &id:oatpp::websocket::Config;.
   */
  void setConfig(const Config& config);

  /**
   * Get WebSocket connection.
   * @return - &id:oatpp::data::stream::IOStream;.
   */
  std::shared_ptr<oatpp::data::stream::IOStream> getConnection() const {
    return m_connection;
  }

  /**
   * Set WebSocket events listener.
   * @param listener - &l:WebSocket::Listener;.
   */
  void setListener(const std::shared_ptr<Listener>& listener) const {
    m_listener = listener;
  }

  /**
   * Get socket event listener
   * @return - &l:WebSocket::Listener;.
   */
  std::shared_ptr<Listener> getListener() const {
    return m_listener;
  }

  /**
   * Read one frame from connection and call corresponding methods of listener. <br>
   * *Use this method if you know what you are doing.*
   * @param frameHeader - &id:oatpp::websocket::Frame::Header;.
   */
  void iterateFrame(Frame::Header& frameHeader);

  /**
   * Blocks until stopListening() is called or error occurred
   * Read incoming frames and call corresponding methods of listener.
   * See &l:WebSocket::setListener ();.
   */
  void listen();
  
  /**
   * Break listen loop. See &l:WebSocket::listen ();.
   */
  void stopListening() const;

  /**
   * Send custom frame to peer. <br>
   * *Use this method if you know what you are doing.*
   * @param frameHeader - &id:oatpp::websocket::Frame::Header;.
   */
  void writeFrameHeader(const Frame::Header& frameHeader) const;

  /**
   * Send default frame to peer with fin, opcode and messageSize set. <br>
   * *Use this method if you know what you are doing.*
   * @param frameHeader - &id:oatpp::websocket::Frame::Header;.
   * @param fin - FIN bit.
   * @param opcode - operation code.
   * @param messageSize - coming message size.
   */
  void sendFrameHeader(Frame::Header& frameHeader, bool fin, v_uint8 opcode, v_int64 messageSize) const;

  /**
   * Send one frame message with custom fin and opcode.
   * @param fin - FIN bit.
   * @param opcode - operation code.
   * @param message - message text. &id:oatpp::String;.
   * @return - `true` on success, `false` on error.
   * if `false` returned socket should be closed manually.
   */
  bool sendOneFrame(bool fin, v_uint8 opcode, const oatpp::String& message) const;

  /**
   * Send close frame.
   * @param code - close message code.
   * @param message - message text. &id:oatpp::String;.
   * @throws - `runtime_error`.
   */
  void sendClose(v_uint16 code, const oatpp::String& message) const;

  /**
   * Send close frame without message.
   * @throws - `runtime_error`.
   */
  void sendClose() const;

  /**
   * Send ping frame.
   * @param message - message text. &id:oatpp::String;.
   * @throws - `runtime_error`.
   */
  void sendPing(const oatpp::String& message) const;

  /**
   * Send pong message.
   * @param message - message text. &id:oatpp::String;.
   * @throws - `runtime_error`.
   */
  void sendPong(const oatpp::String& message) const;

  /**
   * Send one-frame text message.
   * @param message - message text. &id:oatpp::String;.
   * @throws - `runtime_error`.
   */
  void sendOneFrameText(const oatpp::String& message) const;

  /**
   * Send one-frame binary message.
   * @param message - message text. &id:oatpp::String;.
   * @throws - `runtime_error`.
   */
  void sendOneFrameBinary(const oatpp::String& message) const;
  
};
  
}}

#endif /* oatpp_websocket_WebSocket_hpp */
