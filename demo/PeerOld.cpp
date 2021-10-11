#include "Peer.hpp"
#include "Log.hpp"

using namespace std;
using namespace nlohmann;



    Peer::Peer() {

    }
    Peer::~Peer() {

    }
void Peer::notify(string method, json data)
{
    MS_lOGD("Peer notify method=%s data=%s",method.c_str(),data.dump(4).c_str());
  if(protooPeer) {
      protooPeer->notify(method,data);
  }
   
}
void Peer::request(string method, json data)
{
        MS_lOGD("Peer request method=%s data=%s",method.c_str(),data.dump(4).c_str());
    if(protooPeer) {
        protooPeer->request(method,data);
    }
}
