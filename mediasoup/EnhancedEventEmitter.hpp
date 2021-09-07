#pragma once
#include <json.hpp>
#include "Log.hpp"
#include <string>
#include "EventEmitterEx.hpp"
using json = nlohmann::json;
//#include "EventEmitter.hpp"
#include "util/event_emitter.hpp"
//const logger = new Logger('EnhancedEventEmitter');
//https://github.com/HQarroum/event-emitter
namespace mediasoup
{
  
//using ListenerT = std::function< void( const std::string & str) >;
//using EventT = std::string;
class EnhancedEventEmitter  : public EventEmitterEx //<EventT,ListenerT>
{

public:
  EnhancedEventEmitter()
  {
      
  }
 
};

}
