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

#ifndef StatisticsController_hpp
#define StatisticsController_hpp

#include "../dto/ConfigDto.hpp"
#include "../utils/Statistics.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/network/ConnectionHandler.hpp"

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"


#include OATPP_CODEGEN_BEGIN(ApiController) /// <-- Begin Code-Gen

class StatisticsController : public oatpp::web::server::api::ApiController {
private:
  typedef StatisticsController __ControllerType;
private:
  OATPP_COMPONENT(oatpp::Object<ConfigDto>, m_appConfig);
  OATPP_COMPONENT(std::shared_ptr<Statistics>, m_statistics);
public:
  StatisticsController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper)
  {}
public:

  ENDPOINT_ASYNC("GET", m_appConfig->statisticsUrl, Stats) {

    ENDPOINT_ASYNC_INIT(Stats)

    Action act() override {
      auto json = controller->m_statistics->getJsonData();
      auto response = controller->createResponse(Status::CODE_200, json);
      response->putHeader(Header::CONTENT_TYPE, "application/json");
      return _return(response);
    }

  };

};

#include OATPP_CODEGEN_END(ApiController) /// <-- End Code-Gen

#endif /* StatisticsController_hpp */
