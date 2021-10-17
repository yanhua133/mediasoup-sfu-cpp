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

#ifndef ASYNC_SERVER_ROOMS_ROOM_HPP
#define ASYNC_SERVER_ROOMS_ROOM_HPP

#include "./Peer.hpp"
#include "../dto/DTOs.hpp"
#include "../utils/Statistics.hpp"

#include "oatpp/core/macro/component.hpp"

#include <unordered_map>
#include <list>

#include "Router.hpp"

#include "IMediasoup.hpp"
#include "IWorker.hpp"
#include "Log.hpp"
#include "Consumer.hpp"
#include "Router.hpp"
#include "Log.hpp"
#include "../Config.hpp"
#include "../Transfmt.hpp"
#include "AudioLevelObserver.hpp"

class Room {
public:
    //Config
    std::shared_ptr<Config> m_pConfig;
    //record
    Transfmt transfmt;
private:
    oatpp::String m_id;
    std::shared_ptr<Router> m_mediasoupRouter ;//= mediasoupRouter;
    std::unordered_map<std::string, std::shared_ptr<Peer>> m_peers;
    std::list<oatpp::Object<MessageDto>> m_history;
    std::mutex m_peersLock;
    std::mutex m_historyLock;
private:
    OATPP_COMPONENT(oatpp::Object<ConfigDto>, m_appConfig);
    OATPP_COMPONENT(std::shared_ptr<Statistics>, m_statistics);
public:
    
    Room(const oatpp::String& id, std::shared_ptr<Router> mediasoupRouter)
    : m_id(id)
    , m_mediasoupRouter(mediasoupRouter)
    {
        ++ m_statistics->EVENT_ROOM_CREATED;
    }
    
    ~Room() {
        ++ m_statistics->EVENT_ROOM_DELETED;
    }
    
    /**
     * Get room name.
     * @return
     */
    oatpp::String getId();
    
    //set mediasoup config
    void setConfig(std::shared_ptr<Config> pConfig);
    //Add peer to the room.
    void addPeer(const std::shared_ptr<Peer>& peer);
    
    //Inform the audience about the new peer.
    void welcomePeer(const std::shared_ptr<Peer>& peer);
    
    //Send info about other peers and available chat history to peer.
    void onboardPeer(const std::shared_ptr<Peer>& peer);
    
    //Send peer left room message.
    void goodbyePeer(const std::shared_ptr<Peer>& peer);
    
    //Get peer by id.
    std::shared_ptr<Peer> getPeerById(std::string peerId);
    //get joined peer
    std::vector<shared_ptr<Peer> > getJoinedPeers(std::shared_ptr<Peer> excludePeer = nullptr);
    //get joined peer map
    std::unordered_map<std::string,shared_ptr<Peer>> getJoinedPeersMap(std::shared_ptr<Peer> excludePeer = nullptr);
    //create consumer
    void createConsumer(std::shared_ptr<Peer> &consumerPeer, std::shared_ptr<Peer> &producerPeer, std::shared_ptr<Producer>  &producer);
    void createDataConsumer(
                             std::shared_ptr<Peer> &dataConsumerPeer,
                             std::shared_ptr<Peer> dataProducerPeer, // This is null for the bot dataProducer->
                             std::shared_ptr<DataProducer> dataProducer
                            );
    
    
    //Remove peer from the room.
    void removePeerById(std::string peerId);
    
    
    //Add message to history.
    void addHistoryMessage(const oatpp::Object<MessageDto>& message);
    
    //when the request from client arrived
    void handleRequest(std::shared_ptr<Peer> &peer, json &request, std::function<void(json data)> const & accept, std::function<void(int errorCode, std::string errorReason)> const & reject);
    
    //handle notification
    void handleNotification(json notification);
    
    //Get list of history messages.
    oatpp::List<oatpp::Object<MessageDto>> getHistory();
    
    //Send message to all peers in the room.
    void sendMessageAsync(const oatpp::Object<MessageDto>& message);
    
    //Websocket-Ping all peers.
    void pingAllPeers();
    
    //Check if room is empty (no peers in the room).
    bool isEmpty();
    
    void connectBroadcasterTransport(
                                     std::string broadcasterId,
                                     std::string transportId,
                                     DtlsParameters &dtlsParameters
                                     ){}
    
    /**
     * Create a mediasoup Producer associated to a Broadcaster.
     *
     * @async
     *
     * @type {String} broadcasterId
     * @type {String} transportId
     * @type {String} kind - "audio" or "video" kind for the producer->
     * @type {RTCRtpParameters} rtpParameters - RTP parameters for the producer->
     */
    json createBroadcasterProducer(
                                   //{
                                   std::string broadcasterId,
                                   std::string transportId,
                                   std::string kind,
                                   RtpParameters &rtpParameters
    //}
    ){}
    
    json createBroadcasterConsumer(
                                   //{
                                   std::string broadcasterId,
                                   std::string transportId,
                                   std::string producerId
    //}
    ){}
    
    json createBroadcasterDataConsumer(
                                       //{
                                       std::string broadcasterId,
                                       std::string transportId,
                                       std::string dataProducerId
    //}
    ){}
    
    json createBroadcasterDataProducer(
                                       //	{
                                       std::string broadcasterId,
                                       std::string transportId,
                                       std::string label,
                                       std::string protocol,
                                       SctpStreamParameters &sctpStreamParameters,
                                       json appData
    //	}
    ){}
    
    RtpCapabilities getRouterRtpCapabilities()
    {
        return this->m_mediasoupRouter->rtpCapabilities();
    }
    
    json createBridge(std::string id, std::string displayName, json & device, RtpCapabilities &rtpCapabilities)
    {
        
    }
    
    json createBridgeTransport(
                               
                               std::string  bridgeId,
                               std::string type,
                               bool rtcpMux,// = false,
                               bool comedia,// = true,
                               SctpCapabilities &sctpCapabilities
                               )
    {
    }
    
    std::string getBridgeTransportId(std::string &bridgeId) {}
    std::shared_ptr<Transport> getBridgeTransport(std::string &bridgeId){}
    void connectBridgeTransport(
                                std::string &bridgeId,
                                std::string &transportId,
                                DtlsParameters &dtlsParameters
                                ){}
    
    json  getLocalSdp(){}
    
    json createBridgeProducer( std::string &bridgeId,
                              std::string &transportId,
                              std::string &kind,
                              RtpParameters &rtpParameters){}
    std::vector<PeerInfo> createBroadcaster(std::string id, std::string displayName, json  device, RtpCapabilities &rtpCapabilities){}
    std::vector<std::shared_ptr<Producer>> getProducersFromBridge(std::string &bridgeId){}
    
};

#endif //ASYNC_SERVER_ROOMS_ROOM_HPP
