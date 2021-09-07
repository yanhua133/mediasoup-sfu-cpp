#pragma once
#include "json.hpp"
using json = nlohmann::json;
namespace mediasoupclient
{
  
    	json parseScalabilityMode(const std::string& scalabilityMode);
}