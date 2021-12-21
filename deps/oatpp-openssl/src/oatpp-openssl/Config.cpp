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

#include "Config.hpp"

#include "configurer/CertificateFile.hpp"
#include "configurer/PrivateKeyFile.hpp"
#include "configurer/CertificateChainFile.hpp"

namespace oatpp { namespace openssl {

Config::Config() {}

Config::~Config(){}

std::shared_ptr<Config> Config::createShared() {
  return std::make_shared<Config>();
}

std::shared_ptr<Config> Config::createDefaultServerConfigShared(const oatpp::String& certFile,
                                                                const oatpp::String& privateKeyFile,
                                                                const oatpp::String& certChainFile)
{
  auto config = createShared();
  config->addContextConfigurer(std::make_shared<configurer::CertificateFile>(certFile));
  config->addContextConfigurer(std::make_shared<configurer::PrivateKeyFile>(privateKeyFile));
  if(certChainFile) {
    config->addContextConfigurer(std::make_shared<configurer::CertificateChainFile>(certChainFile));
  }
  return config;
}

std::shared_ptr<Config> Config::createDefaultClientConfigShared() {
  return createShared();
}

void Config::clearContextConfigurers() {
  m_contextConfigs.clear();
}

void Config::addContextConfigurer(const std::shared_ptr<configurer::ContextConfigurer>& contextConfigurer) {
  m_contextConfigs.push_back(contextConfigurer);
}

void Config::configureContext(SSL_CTX* ctx) const {
  for(auto& c : m_contextConfigs) {
    c->configure(ctx);
  }
}
  
}}
