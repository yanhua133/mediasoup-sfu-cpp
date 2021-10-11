/***************************************************************************
 *
 * Project:   ______                ______ _
 *           / _____)              / _____) |          _
 *          | /      ____ ____ ___| /     | | _   ____| |_
 *          | |     / _  |  _ (___) |     | || \ / _  |  _)
 *          | \____( ( | | | | |  | \_____| | | ( ( | | |__
 *           \______)_||_|_| |_|   \______)_| |_|\_||_|\___)
 *
 *
 * Copyright 2020-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
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

#ifndef RoomsController_hpp
#define RoomsController_hpp

#include "utils/Nickname.hpp"

#include "oatpp-websocket/Handshaker.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/network/ConnectionHandler.hpp"

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"


#include OATPP_CODEGEN_BEGIN(ApiController) /// <-- Begin Code-Gen

class RoomsController : public oatpp::web::server::api::ApiController {
private:
  typedef RoomsController __ControllerType;
private:
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, websocketConnectionHandler, "websocket");
public:
  RoomsController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper)
  {}
public:

  //this._protooUrl = `ws://192.168.0.106:8001/?roomId=${roomInfo.roomId}&peerId=${roomInfo.userId}&forceH264=true&forceVP9=false`
  ENDPOINT_ASYNC("GET", "/", WS,
                 QUERY(String, roomId, "roomId", "123456"),
                 QUERY(String, peerId, "peerId", "123456"),
                 QUERY(Bool, forceH264, "forceH264", false),
                 QUERY(Bool, forceVP9, "forceVP9", false)) {

    ENDPOINT_ASYNC_INIT(WS)

    Action act() override {
      /* Websocket handshake */
      auto response = oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), controller->websocketConnectionHandler);

      auto parameters = std::make_shared<oatpp::network::ConnectionHandler::ParameterMap>();

      (*parameters)["roomId"] = roomId;
      (*parameters)["peerId"] = peerId;
      (*parameters)["forceH264"] = forceH264;
      (*parameters)["forceVP9"] = forceVP9;

      /* Set connection upgrade params */
      response->setConnectionUpgradeParameters(parameters);

      return _return(response);

    }

  };

};

#include OATPP_CODEGEN_END(ApiController) /// <-- End Code-Gen

#endif /* RoomsController_hpp */

