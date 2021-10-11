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

#ifndef oatpp_websocket_Handshaker_hpp
#define oatpp_websocket_Handshaker_hpp

#include "./WebSocket.hpp"

#include "oatpp/web/protocol/http/outgoing/Response.hpp"
#include "oatpp/web/protocol/http/outgoing/Request.hpp"
#include "oatpp/web/protocol/http/incoming/Response.hpp"
#include "oatpp/web/protocol/http/incoming/Request.hpp"

#include <random>

namespace oatpp { namespace websocket {

/**
 * Helper class providing WebSocket handshake functionality.
 */
class Handshaker {
public:
  static const char* const MAGIC_UUID;
public:
  /**
   * Handshake OK.
   */
  static constexpr v_int32 STATUS_OK = 0;

  /**
   * Server response-code != 101.
   */
  static constexpr v_int32 STATUS_SERVER_ERROR = 1;

  /**
   * Server response "Sec-WebSocket-Accept" header is wrong.
   */
  static constexpr v_int32 STATUS_SERVER_WRONG_KEY = 2;

  /**
   * Server's response contains unexpected headers values
   */
  static constexpr v_int32 STATUS_UNKNOWN_PROTOCOL_SUGGESTED = 3;

public:
  /**
   * Convenience typedef for &id:oatpp::web::protocol::http::outgoing::Response;.
   */
  typedef oatpp::web::protocol::http::outgoing::Response OutgoingResponse;

  /**
   * Convenience typedef for &id:oatpp::web::protocol::http::incoming::Response;.
   */
  typedef oatpp::web::protocol::http::incoming::Response IncomingResponse;
  typedef oatpp::web::protocol::http::Headers Headers;
  typedef oatpp::network::ConnectionHandler ConnectionHandler;
public:

  /**
   * Prepare OutgoingResponse as for websocket-handshake based on requestHeaders.
   * @param requestHeaders - request headers. &id:oatpp::web::protocol::http::Headers;.
   * @param connectionUpgradeHandler - &id:oatpp::network::ConnectionHandler;.
   * @return - `std::shared_ptr` to &id:oatpp::web::protocol::http::outgoing::Response;.
   */
  static std::shared_ptr<OutgoingResponse> serversideHandshake(const Headers& requestHeaders, const std::shared_ptr<ConnectionHandler>& connectionUpgradeHandler);

  /**
   * Set client request headers as for websocket-handshake.
   * @param requestHeaders - headers map. &id:oatpp::web::protocol::http::Headers;.
   */
  static void clientsideHandshake(Headers& requestHeaders);

  /**
   * Check if client's handshake corresponds to server's handshake
   * @param clientHandshakeHeaders - previously prepared headers which were sent to server.
   * &id:oatpp::web::protocol::http::Headers;.
   * See &l:Handshaker::clientsideHandshake ();.
   * @param serverResponse - &id:oatpp::web::protocol::http::incoming::Response;.
   * @return - one of:
   * <ul>
   *   <li>&l:Handshaker::STATUS_OK;</li>
   *   <li>&l:Handshaker::STATUS_SERVER_ERROR;</li>
   *   <li>&l:Handshaker::STATUS_SERVER_WRONG_KEY;</li>
   *   <li>&l:Handshaker::STATUS_UNKNOWN_PROTOCOL_SUGGESTED;</li>
   * </ul>
   */
  static v_int32 clientsideConfirmHandshake(const Headers& clientHandshakeHeaders, const std::shared_ptr<IncomingResponse>& serverResponse);
  
};
  
}}

#endif /* oatpp_websocket_Handshaker_hpp */
