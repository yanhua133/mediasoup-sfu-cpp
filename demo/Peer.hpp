#ifndef _PEER_H
#define _PEER_H

#include "json.hpp"
#include "Transport/WebSocketTransport.h"
#include <stdexcept>
#include "utils/EventEmitter.hpp"
#include "Peer.h"
#include "Consumer.hpp"
#include "Producer.hpp"
#include "DataProducer.hpp"
#include "DataConsumer.hpp"
#include "Transport.hpp"

using namespace std;
using namespace nlohmann;
struct _device
{
    std::string flag ;//   : "broadcaster",
    std::string name ;//   : device.name || "Unknown device",
    std::string version ;//: device.version
};

struct PeerInfo
{
    std::string id;
    std::string displayName;
    _device device;
    json producerInfo;
    std::vector<json> producers;
 
};
class PeerData
{
  public:
  std::string id;

  std::shared_ptr<Consumer>  consume;
  bool joined;
  std::string displayName;
  _device device;
  json rtpCapabilities;
  json sctpCapabilities;

  std::unordered_map<std::string,std::shared_ptr<Transport> > transports;
  std::unordered_map<std::string,std::shared_ptr<Producer> > producers;
  std::unordered_map<std::string,std::shared_ptr<Consumer> > consumers;
  std::unordered_map<std::string,std::shared_ptr<DataProducer> > dataProducers;
  std::unordered_map<std::string,std::shared_ptr<DataConsumer> > dataConsumers;

};

class Peer {
    public:
        Peer();
        ~Peer();
      void notify(string method, json data = json({}));
      void request(string method, json data = json({}));
    public:
        std::string id;
        PeerData data;
 
    bool isHasRequest = false;
    bool isHasNotify = true;
    public:
       std::shared_ptr<protoo::Peer> protooPeer;
  

};


#endif

