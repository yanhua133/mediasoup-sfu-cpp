//
// Created by pan chenguang on 2021/2/4.
//

#ifndef TESTBEASTCLION_ROOM_H
#define TESTBEASTCLION_ROOM_H
#include "utils/EventEmitter.hpp"
#include <string>
#include <map>
#include <iostream>
#include "Peer.h"

using namespace std;

namespace protoo {
    class Room : public EventEmitter {
    public:
        Room();
        ~Room();
    public:
        bool closed();
        void close();
        std::map<string,shared_ptr<Peer>>& getPeers();
        std::shared_ptr<Peer> createPeer(string peerId, WebSocketTransport* transport);
        void  removePeer(string peerId);//add by jacky 20211013
        bool hasPeer(string peerId);
        shared_ptr<Peer> getPeer(string peerId);
    private:
        // Map of Peers indexed by id
        std::map<string,shared_ptr<Peer>> m_peers;
        //whether room was closed
        bool m_closed;
        // This mutex synchronizes all access to WebSocketTransport
        std::mutex mutex_;
    };
}


#endif //TESTBEASTCLION_ROOM_H
