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

#ifndef oatpp_openssl_Config_hpp
#define oatpp_openssl_Config_hpp

#include "oatpp-openssl/configurer/ContextConfigurer.hpp"

#include "oatpp/core/Types.hpp"

namespace oatpp { namespace openssl {

class Config {
private:
  std::list<std::shared_ptr<configurer::ContextConfigurer>> m_contextConfigs;
public:

  Config();

  virtual ~Config();

  static std::shared_ptr<Config> createShared();

  static std::shared_ptr<Config> createDefaultServerConfigShared(const oatpp::String& certFile,
                                                                 const oatpp::String& privateKeyFile,
                                                                 const oatpp::String& certChainFile = nullptr);

  static std::shared_ptr<Config> createDefaultClientConfigShared();

  void clearContextConfigurers();
  void addContextConfigurer(const std::shared_ptr<configurer::ContextConfigurer>& contextConfigurer);

  void configureContext(SSL_CTX* ctx) const;

};
  
}}

#endif /* oatpp_openssl_Config_hpp */
