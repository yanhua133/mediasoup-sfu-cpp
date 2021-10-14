//
// Created by pan chenguang on 2021/2/4.
//

#include "Room.h"
#include "utils/Log.hpp"
namespace protoo {
    Room::Room() {
      this->m_closed = false;
    }

    Room::~Room() {

    }

    //whether the Room is closed.
    bool Room::closed()
    {
        return this->m_closed;
    }

    std::map<string,shared_ptr<Peer>>& Room::getPeers()
    {
        return m_peers;
    }

    //close the room
    void Room::close()
    {
        if(this->closed())
        {
            return;
        }
        std::cout<<"Room close()"<<std::endl;
        std::lock_guard<std::mutex> lock(mutex_);
        for(auto& it : this->m_peers)
        {
            it.second->close();
        }
    }

    std::shared_ptr<Peer> Room::createPeer(string peerId, WebSocketTransport* transport)
    {
       if(transport == nullptr)
       {
           std::cout<<"[Room] no transport given"<<std::endl;
           return nullptr;
       }
       if(m_peers.find(peerId)!=m_peers.end())
       {
           cout<<"there is already a Peer with same peerId:"<<peerId<<endl;
           return nullptr;
       }
       //Create the Peer instance
        std::lock_guard<std::mutex> lock(mutex_);
       shared_ptr<Peer> peer(new Peer(peerId, transport));
       //Peer peer(peerId, transport);
        // Store it in the map.
        this->m_peers.emplace(peer->id(),peer);
        // TODO
        // peer->on("close", [&]() {
        //     this->m_peers.erase(peer->id());
        // });
        return peer;
    }
    void  Room::removePeer(string peerId)
    {
        this->m_peers.erase(peerId);
    }
    bool Room::hasPeer(string peerId)
    {
        return this->m_peers.find(peerId)!=m_peers.end();
    }

    shared_ptr<Peer> Room::getPeer(string peerId)
    {
        if (m_peers.find(peerId) == m_peers.end()) //add by jacky 20211014
        {
            cout << "get Peer with same = " << peerId << "return null!" << endl;
            return nullptr;
        }

        return this->m_peers[peerId];
    }


}