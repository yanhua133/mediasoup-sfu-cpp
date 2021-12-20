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

#include "Room.hpp"

oatpp::String Room::getId() {
  return m_id;
}

void Room::addPeer(const std::shared_ptr<Peer>& peer) {
  std::lock_guard<std::mutex> guard(m_peersLock);
  m_peers[peer->getPeerId()] = peer;
}

void Room::welcomePeer(const std::shared_ptr<Peer>& peer) {

  /* Inform all that peer have joined the room */

//  auto joinedMessage = MessageDto::createShared();
//  joinedMessage->code = MessageCodes::CODE_PEER_JOINED;
//  joinedMessage->peerId = oatpp::String(peer->getPeerId().c_str());
//  joinedMessage->peerName = peer->getNickname();
//  joinedMessage->message = peer->getNickname() + " - joined room";
//
//  addHistoryMessage(joinedMessage);
  // sendMessageAsync(joinedMessage);

}

void Room::setConfig(std::shared_ptr<Config> pConfig) {
    this->m_pConfig = pConfig;
}

void Room::onboardPeer(const std::shared_ptr<Peer>& peer) {

//  auto infoMessage = MessageDto::createShared();
//  infoMessage->code = MessageCodes::CODE_INFO;
//  //infoMessage->peerId = peer->getPeerId();
//  infoMessage->peerName = peer->getNickname();
//
//  infoMessage->peers = {};
//
//  {
//    std::lock_guard<std::mutex> guard(m_peerByIdLock);
//    for (auto &it : m_peerById) {
//      auto p = PeerDto::createShared();
////      p->peerId = it.second->getPeerId();
////      p->peerName = it.second->getNickname();
////      infoMessage->peers->push_back(p);
//    }
//  }
//
//  infoMessage->history = getHistory();
//  peer->sendMessageAsync(infoMessage);

}

void Room::goodbyePeer(const std::shared_ptr<Peer>& peer) {

//  auto message = MessageDto::createShared();
//  message->code = MessageCodes::CODE_PEER_LEFT;
//  //message->peerId = peer->getPeerId();
//  message->message = peer->getNickname() + " - left room";
//
//  addHistoryMessage(message);
//  sendMessageAsync(message);

}

std::shared_ptr<Peer> Room::getPeerById(std::string peerId) {
  std::lock_guard<std::mutex> guard(m_peersLock);
  auto it = m_peers.find(peerId);
  if(it != m_peers.end()) {
    return it->second;
  }
  return nullptr;
}

void Room::removePeerById(std::string peerId) {

  std::lock_guard<std::mutex> guard(m_peersLock);
  auto peer = m_peers.find(peerId);

  if(peer != m_peers.end()) {
    m_peers.erase(peerId);

  }

}

void Room::addHistoryMessage(const oatpp::Object<MessageDto>& message) {

  if(!m_appConfig->maxRoomHistoryMessages || *m_appConfig->maxRoomHistoryMessages == 0) {
    return;
  }

  std::lock_guard<std::mutex> guard(m_historyLock);

  m_history.push_back(message);

  while(m_history.size() > m_appConfig->maxRoomHistoryMessages) {
    m_history.pop_front();
  }

}

void Room::handleRequest(std::shared_ptr<Peer> &peer, json &request, std::function<void(json data)> const & accept, std::function<void(int errorCode, std::string errorReason)> const & reject){
    std::string method = request["method"];
    //print current thread
    std::ostringstream ss;
    ss << std::this_thread::get_id();
    MS_lOGD("[Room] handleRequest current thread:%s",ss.str().c_str());
    //switch (request.method)
    //MS_lOGD("_handleProtooRequest request data = %s",request["data"].dump().c_str());
    if(!m_pConfig){
        MS_lOGD("[Room] handleRequest config not existing");
    }
    if(method == "getRouterRtpCapabilities")
    {
        json rtpCapabilities = this->m_mediasoupRouter->rtpCapabilities();
        MS_lOGE("[Room] handleRequest getRouterRtpCapabilities peerId=%s, peerName=%s, rtpCapabilities = 暂不显示",peer->getPeerId().c_str(),peer->getNickname().c_str()); //rtpCapabilities.dump(4).c_str());
        accept(rtpCapabilities);
    }else if(method == "join"){
        // Ensure the Peer is not already joined.
        if (peer->data.joined)
        {
            MS_THROW_lOG("[Room] Peer already joined");
            // accept(json({}));
            // return;
        }
        auto data = request["data"];
        auto displayName = data["displayName"];
        auto device = data["device"];
        auto rtpCapabilities = data["rtpCapabilities"];
        auto sctpCapabilities = data["sctpCapabilities"];
        peer->setNickname(displayName);
        MS_lOGD("[Room] peer join peerId=%s,displayName=%s,rtpCapabilities=%s",peer->getPeerId().c_str(), peer->getNickname().c_str(), rtpCapabilities.dump(4).c_str());
        
        // Store client data into the protoo Peer data object.
        peer->data.joined = true;
        peer->data.displayName = displayName;
        peer->data.device = device;
        peer->data.rtpCapabilities = rtpCapabilities;
        peer->data.sctpCapabilities = sctpCapabilities;
        
        // Tell the new Peer about already joined Peers.
        // And also create Consumers for existing Producers.
        
        // const joinedPeers =
        // [
        //     ...this->_getJoinedPeers(),
        //     ...this->_broadcasters,
        //     ...this->_bridges
        // ];
        std::unordered_map<std::string,std::shared_ptr<Peer>> joinedPeers;
        std::unordered_map<std::string, shared_ptr<Peer>> joinPeers = this->getJoinedPeersMap();
        joinedPeers.insert(joinPeers.begin(), joinPeers.end());
        //joinedPeers.insert(this->_broadcasters.begin(), this->_broadcasters.end());
        //joinedPeers.insert(this->_bridges.begin(), this->_bridges.end());
        
        // Reply now the request with the list of joined peers (all but the new one).
        auto it = joinedPeers.find(peer->getPeerId());
        if ( it != joinedPeers.end())
        {
            joinedPeers.erase (it);
        }
        
        json peerInfos = json::array();
        for( const auto& kv  : joinedPeers)
        {
            auto    peerId      = kv.first;
            auto  &joinedPeer    = kv.second;
            
            peerInfos.push_back({
                {"id"          , joinedPeer->getPeerId()},
                {"displayName" , joinedPeer->data.displayName},
                {"device"      , joinedPeer->data.device}
            });
        }
        // const peerInfos = joinedPeers
        //     .filter((joinedPeer) => joinedPeer->id != peer->id)
        //     .map((joinedPeer) => ({
        //         id          : joinedPeer->id(),
        //         displayName : joinedPeer->data.displayName,
        //         device      : joinedPeer->data.device
        //     }));
        
        accept({{ "peers", peerInfos }});
        
        // Mark the new Peer as joined.
        peer->data.joined = true;
        
        for (auto &kv : joinedPeers)
        {
            auto &joinedPeer = kv.second;
            // Create Consumers for existing Producers.
            for (auto &kv1 : joinedPeer->data.producers)
            {
                auto &producer = kv1.second;
                this->createConsumer(
                                     /*consumerPeer :*/ peer,
                                     /*producerPeer :*/ joinedPeer,
                                     producer
                                     );
            }
            
            // Create DataConsumers for existing DataProducers.
            for (auto &kv2 : joinedPeer->data.dataProducers)
            {
                auto &dataProducer = kv2.second;
                if (dataProducer->label() == "bot")
                    continue;
                
                this->createDataConsumer(
                                         /*dataConsumerPeer :*/ peer,
                                         /*dataProducerPeer :*/ joinedPeer,
                                         dataProducer
                                         );
            }
        }
        
        // Create DataConsumers for bot dataProducer->
        this->createDataConsumer(
                                 
                                 /*dataConsumerPeer :*/ peer,
                                 /*dataProducerPeer :*/ nullptr,
                                 /*dataProducer     :*/ nullptr//this->_bot.dataProducer
                                 );
        
        // Notify the new Peer to all other Peers.
        for (auto  &otherPeer : this->getJoinedPeers( peer ))
        {
            if(!otherPeer)
            {
                continue;
            }
            otherPeer->notifyAsync(
                                   "newPeer",
                                   {
                {"id"          , peer->getPeerId()},
                {"displayName" , peer->data.displayName},
                {"device"      , peer->data.device}
            });
            json jsonmsg={
                {"id"          , peer->getPeerId()},
                {"displayName" , peer->data.displayName},
                {"device"      , peer->data.device}
            };
            MS_lOGD("[Room] on peer join otherPeer peerId=%s notifyAsync=%s",otherPeer->getPeerId().c_str(),jsonmsg.dump(4).c_str());
        }
        
        //break;
    }else if(method ==  "createWebRtcTransport"){
        // NOTE: Don"t require that the Peer is joined here, so the client can
        // initiate mediasoup Transports and be ready when he later joins.
        
        // const {
        //     forceTcp,
        //     producing,
        //     consuming,
        //     sctpCapabilities
        // } = request.data;
        auto data = request["data"];
        bool forceTcp = data["forceTcp"];
        bool producing = data["producing"];
        bool consuming = data["consuming"];
        json sctpCapabilities = data["sctpCapabilities"];
        MS_lOGD("createWebRtcTransport request.data=%s",data.dump().c_str());
        
        json jwebRtcTransportOptions = m_pConfig->mediasoup.webRtcTransportOptions;
        jwebRtcTransportOptions["appData"] = { {"producing",producing}, {"consuming",consuming }};
        if(sctpCapabilities.is_object() && !sctpCapabilities["numStreams"].is_null()){
            jwebRtcTransportOptions["enableSctp"] = true;
            jwebRtcTransportOptions["numSctpStreams"] = sctpCapabilities["numStreams"];
        }else{
            jwebRtcTransportOptions["enableSctp"] = false;//!sctpCapabilities.dump().empty();
            jwebRtcTransportOptions["numSctpStreams"] = json::object();
        }
        // const webRtcTransportOptions =
        // {
        //     ...config.mediasoup.webRtcTransportOptions,
        //     enableSctp     : Boolean(sctpCapabilities),
        //     numSctpStreams : (sctpCapabilities || {}).numStreams,
        //     appData        : { producing, consuming }
        // };
        WebRtcTransportOptions webRtcTransportOptions = jwebRtcTransportOptions;
        if (forceTcp)
        {
            webRtcTransportOptions.enableUdp = false;
            webRtcTransportOptions.enableTcp = true;
        }
        auto transport =  this->m_mediasoupRouter->createWebRtcTransport(webRtcTransportOptions);
        transport->on("sctpstatechange", []( std::string sctpState ) //(sctpState) =>
                      {
            MS_lOGD("WebRtcTransport sctpstatechange event [sctpState:%s]", sctpState.c_str());
        });
        
        transport->on("dtlsstatechange", []( std::string dtlsState ) //(dtlsState) =>
                      {
            
            if (dtlsState == "failed" || dtlsState == "closed")
                MS_lOGW("WebRtcTransport dtlsstatechange event [dtlsState:%s]", dtlsState.c_str());
        });
        
        // NOTE: For testing.
        //  transport->enableTraceEvent([ "probation", "bwe" ]);
        std::vector<std::string> types;
        types.push_back("probation");
        types.push_back("bwe");
        transport->enableTraceEvent(types);
        
        transport->on("trace", [peer, transport]( TransportTraceEventData & trace ) //(trace) =>
                      {
            
            MS_lOGD(
                    "transport trace event [transportId:%s, trace.type:%s, trace]",
                transport->id().c_str(), trace.type.c_str());
            
            if (trace.type == "bwe" && trace.direction == "out")
            {
                peer->notifyAsync(
                                  "downlinkBwe",
                                  {
                    {"desiredBitrate"          , trace.info["desiredBitrate"]},
                    {"effectiveDesiredBitrate" , trace.info["effectiveDesiredBitrate"]},
                    {"availableBitrate"        , trace.info["availableBitrate"]}
                });
                
            }
        });
        
        // Store the WebRtcTransport into the protoo Peer data Object.
        peer->data.transports[transport->id()]= transport;
        json jiceCandidates=transport->iceCandidates();
        MS_lOGD("iceCandidates=%s",jiceCandidates.dump(4).c_str());
        json jdtlsParameters=transport->dtlsParameters();
        MS_lOGD("dtlsParameters=%s",jdtlsParameters.dump(4).c_str());
        if(transport->sctpParameters().port == 0)
        {
            json sendata = {
                {"id"             , transport->id()},
                {"iceParameters"  , transport->iceParameters()},
                {"iceCandidates"  , transport->iceCandidates()},
                {"dtlsParameters" , transport->dtlsParameters()}
                
            };
            MS_lOGD("sendata=%s",sendata.dump(4).c_str());
            accept(sendata
                   );
        } else {
            json sendata = {
                {"id"             , transport->id()},
                {"iceParameters"  , transport->iceParameters()},
                {"iceCandidates"  , transport->iceCandidates()},
                {"dtlsParameters" , transport->dtlsParameters()},
                {"sctpParameters" , transport->sctpParameters()}
            };
            MS_lOGD("sendata=%s",sendata.dump(4).c_str());
            accept(sendata
                   );
        }
        
        
        //const { maxIncomingBitrate } = config.mediasoup.webRtcTransportOptions;
        auto &maxIncomingBitrate = m_pConfig->mediasoup.webRtcTransportOptions["maxIncomingBitrate"];
        // If set, apply max incoming bitrate limit.
        if (maxIncomingBitrate != 0)
        {
            try {
                transport->setMaxIncomingBitrate(maxIncomingBitrate);
                
            }
            catch (const char *error) {
                
            }
        }
        
        //break;
    }else if(method ==  "connectWebRtcTransport"){
        auto data = request["data"];
        auto transportId = data["transportId"];
        auto dtlsParameters = data["dtlsParameters"];
        auto transport = peer->data.transports[transportId];
        
        if (!transport)
            MS_THROW_lOG("transport with id transportId=%s not found",transportId.dump().c_str());
        
        json newdata= json::object();
        newdata["dtlsParameters"] = dtlsParameters;
        MS_lOGD("connectWebRtcTransport dtlsParameters=%s",newdata.dump(4).c_str());
        transport->connect(newdata);
        
        accept(json({}));
    }else if(method ==  "restartIce"){
        auto data = request["data"];
        auto transportId = data["transportId"];
        auto transport = peer->data.transports[transportId];
        
        if (!transport)
        {
            MS_THROW_lOG("transport with id transportId=%s not found",transportId.dump().c_str());
            accept(json({}));
            return;
        }
        
        auto iceParameters =  transport->restartIce();
        
        accept(iceParameters);
        
        
    }else if(method ==  "produce"){
        // Ensure the Peer is joined.
        if (!peer->data.joined)
        {
            MS_THROW_lOG("Peer not yet joined");
            accept(json({}));
            return ;
        }
        auto data = request["data"];
        auto & transportId = data["transportId"];
        auto & kind = data["kind"];
        auto & rtpParameters = data["rtpParameters"];
        auto appData  = data["appData"];
        auto & paused  = data["paused"];
        auto &transport = peer->data.transports[transportId];
        MS_lOGD("produce rtpParameters=%s",rtpParameters.dump(4).c_str());
        if (!transport)
        {
            MS_THROW_lOG("transport with id transportId=%s not found",transportId.dump().c_str());
            return;
        }
        
        
        // Add peerId into appData to later get the associated Peer during
        // the "loudest" event of the audioLevelObserver.
        //appData = { ...appData, peerId: peer->id };
        appData["peerId"] = peer->getPeerId();
        ProducerOptions options;// = {
        options.kind = kind;
        options.rtpParameters = rtpParameters;
        options.appData = appData;
        options.keyFrameRequestDelay = 5000;
        //};
        json jrtpParameters = options.rtpParameters;
        MS_lOGD("produce jrtpParameters=%s",jrtpParameters.dump(4).c_str());
        auto producer =  transport->produce(options);
        
        // Store the Producer into the protoo Peer data Object.
        peer->data.producers[producer->id()]= producer;
        
        // Set Producer events.
        producer->on("score", [peer, producer]( int score) //(score) =>
                     {
            
            // MS_lOGD(
            //     "producer "score" event [producerId:%s, score:%o]",
            //     producer->id(), score);
            
            peer->notifyAsync("producerScore", { {"producerId", producer->id()},{ "score",score} });
            
        });
        
        producer->on("videoorientationchange",[producer]( int videoOrientation ) // (videoOrientation) =>
                     {
            
            MS_lOGD(
                    "producer videoorientationchange event [producerId:%s, videoOrientation:%o]",
                producer->id().c_str(), videoOrientation);
        });
        
        // NOTE: For testing.
        //  producer->enableTraceEvent([ "rtp", "keyframe", "nack", "pli", "fir" ]);
        //  producer->enableTraceEvent([ "pli", "fir" ]);
        //  producer->enableTraceEvent([ "keyframe" ]);
        
        producer->on("trace", [producer](TransportTraceEventData & trace) //(trace) =>
                     {
            
            MS_lOGD(
                    "producer trace event [producerId:%s, trace.type:%s, trace]",
                producer->id().c_str(), trace.type.c_str());
        });
        
        accept({ {"id", producer->id() } });
        
        // auto joinedPeers =
        //     [
        //         ...this->_getJoinedPeers(peer),
        //         ...this->_broadcasters
        //     ];
        std::unordered_map<std::string,std::shared_ptr<Peer>> joinedPeers;
        std::unordered_map<std::string, shared_ptr<Peer>> joinPeers = this->getJoinedPeersMap(peer);
        joinedPeers.insert(joinPeers.begin(), joinPeers.end());
        //joinedPeers.insert(this->_broadcasters.begin(), this->_broadcasters.end());
        
        // Optimization: Create a server-side Consumer for each Peer.
        if(kind == "audio") {
            auto tt=0;
            tt=1;
            
        }else{
            
        }
        for (auto  &kv : joinedPeers)
        {
            auto otherPeer = kv.second;
            if(otherPeer->getPeerId() == peer->getPeerId()) {
                continue;
            }
            this->createConsumer(
                                 
                                 /*consumerPeer :*/ otherPeer,
                                 /*producerPeer :*/ peer,
                                 producer
                                 );
        }
        // 这块需要单独处理，因为需要采集全音频和视频consumer，但是js的设置好像有点延迟，音频设置consumer后但是外层变量没有显示，所以只能先放到这个函数里实现
        //                    for (auto  &kv : this->_bridges)
        //                    {
        //                        auto otherPeer = kv.second;
        //                        if(otherPeer->id == peer->id) {
        //                            continue;
        //                        }
        //                        this->_createBridgeConsumer(
        //
        //                                /*consumerPeer :*/ otherPeer,
        //                                /*producerPeer :*/ peer,
        //                                producer
        //                            );
        //
        //                    }
        if( paused == true) {
            producer->pause();
        }
        // Add into the audioLevelObserver.
        if (producer->kind() == "audio")
        {
            //this->_audioLevelObserver->addProducer(producer->id());
            
        }
        bool hasVideo = false;
        bool hasAudio = false;
        for (auto &kv : peer->data.producers) {
            auto &producer = kv.second;
            if(producer->kind() == "video") {
                hasVideo = true;
            }
            if(producer->kind() == "audio") {
                hasAudio = true;
            }
        }
        //process bridge send offer
        if(hasVideo && hasAudio) {
            // Optimization: Create a server-side Consumer for each Peer.
            //for (auto  otherPeer : this->_bridges)
            //{
            //    this->sendSdpOfferToBridge(otherPeer->id(),peer->id());
            //}
            
        }
        //                    if(this->dvr) {
        //                      if(hasVideo && hasAudio) {
        //                        //mcuPublish(this->_mediasoupRouter, peer);
        //                        //startRecord(this->_mediasoupRouter, peer, producer,this->_roomid());
        //                      }
        //                    }
        
        //break;
    }else if(method ==  "closeProducer"){
        // Ensure the Peer is joined.
        MS_lOGD("closeProducer");
        if (!peer->data.joined)
            MS_THROW_lOG("Peer not yet joined");
        auto data = request["data"];
        auto producerId = data["producerId"];
        auto producer = peer->data.producers[producerId];
        
        if (!producer)
        {
            MS_THROW_lOG("producer with id producerId=%s not found",producerId.dump().c_str());
            return;
        }
        
        bool hasVideo = false;
        bool hasAudio = false;
        if(producer->kind() == "video") {
            hasVideo = true;
        }
        if(producer->kind() == "audio") {
            hasAudio = true;
        }
        producer->close();
        
        // Remove from its map.
        peer->data.producers.erase(producer->id());
        
        accept(json({}));
        //                    if(this->dvr) {
        //                      if(hasAudio) {
        //                          transfmt.stopRecord(peer->id);
        //                      }
        //
        //                    }
        //break;
    }else if(method ==  "pauseProducer"){
        // Ensure the Peer is joined.
        if (!peer->data.joined)
            MS_THROW_lOG("Peer not yet joined");
        auto data = request["data"];
        auto producerId = data["producerId"];
        auto producer = peer->data.producers[producerId];
        
        if (!producer)
            MS_THROW_lOG("producer with id producerId=%s not found",producerId.dump().c_str());
        
        producer->pause();
        
        accept(json({}));
        
        //break;
    }else if(method ==  "resumeProducer"){
        // Ensure the Peer is joined.
        if (!peer->data.joined)
            MS_THROW_lOG("Peer not yet joined");
        auto data = request["data"];
        auto & producerId = data["producerId"];
        auto &producer = peer->data.producers[producerId];
        
        if (!producer)
        {
            MS_THROW_lOG("producer with id producerId=%s not found",producerId.dump().c_str());
            accept(json({}));
            return;
        }
        producer->resume();
        accept(json({}));
    }else if(method ==  "pauseConsumer"){
        // Ensure the Peer is joined.
        if (!peer->data.joined)
            MS_THROW_lOG("Peer not yet joined");
        auto data = request["data"];
        auto & consumerId = data["consumerId"];
        auto  &consumer = peer->data.consumers[consumerId];
        
        if (!consumer)
        {
            MS_THROW_lOG("consumer with id consumerId=%s not found",consumerId.dump().c_str());
            accept(json({}));
            return;
        }
        
        consumer->pause();
        
        accept(json({}));
        
        //break;
    }else if(method ==  "resumeConsumer"){
        // Ensure the Peer is joined.
        if (!peer->data.joined)
            MS_THROW_lOG("Peer not yet joined");
        auto data = request["data"];
        auto & consumerId = data["consumerId"];
        auto  &consumer = peer->data.consumers[consumerId];
        
        if (!consumer)
        {
            MS_THROW_lOG("consumer with id consumerId=%s not found",consumerId.dump().c_str());
            accept(json({}));
            return;
        }
        
        consumer->resume();
        
        accept(json({}));
        
        //break;
    }else if(method ==  "setConsumerPreferredLayers"){
        // Ensure the Peer is joined.
        if (!peer->data.joined)
            MS_THROW_lOG("Peer not yet joined");
        auto data = request["data"];
        auto & consumerId  = data["consumerId"];
        auto & spatialLayer = data["spatialLayer"];
        auto & temporalLayer = data["temporalLayer"];
        auto  &consumer = peer->data.consumers[consumerId];
        
        if (!consumer)
        {
            MS_THROW_lOG("consumer with id consumerId=%s not found",consumerId.dump().c_str());
            accept(json({}));
            return;
        }
        
        consumer->setPreferredLayers(spatialLayer, temporalLayer);
        
        accept(json({}));
        
        //break;
    }else if(method ==  "setConsumerPriority"){
        // Ensure the Peer is joined.
        if (!peer->data.joined)
            MS_THROW_lOG("Peer not yet joined");
        
        auto data = request["data"];
        auto & consumerId  = data["consumerId"];
        auto & priority = data["priority"];
        auto  &consumer = peer->data.consumers[consumerId];
        
        if (!consumer)
        {
            MS_THROW_lOG("consumer with id consumerId=%s not found",consumerId.dump().c_str());
            accept(json({}));
            return;
        }
        
        consumer->setPriority(priority);
        
        accept(json({}));
        
        //break;
    }else if(method ==  "requestConsumerKeyFrame"){
        // Ensure the Peer is joined.
        if (!peer->data.joined)
            MS_THROW_lOG("Peer not yet joined");
        auto data = request["data"];
        auto & consumerId = data["consumerId"];
        auto  &consumer = peer->data.consumers[consumerId];
        
        if (!consumer)
        {
            MS_THROW_lOG("consumer with id consumerId=%s not found",consumerId.dump().c_str());
            accept(json({}));
            return;
        }
        
        consumer->requestKeyFrame();
        
        accept(json({}));
        
        //break;
    }else if(method ==  "produceData"){
        // Ensure the Peer is joined.
        if (!peer->data.joined)
            MS_THROW_lOG("Peer not yet joined");
        
        // const {
        //     transportid(),
        //     sctpStreamParameters,
        //     label,
        //     protocol,
        //     appData
        // } = request.data;
        auto data = request["data"];
        auto & transportId = data["transportId"];
        auto & sctpStreamParameters = data["sctpStreamParameters"];
        auto & label = data["label"];
        auto & protocol = data["protocol"];
        auto & appData = data["appData"];
        
        auto &transport = peer->data.transports[transportId];
        
        if (!transport)
        {
            MS_THROW_lOG("transport with id transportId=%s not found",transportId.dump().c_str());
            accept(json({}));
            return;
        }
        DataProducerOptions option;
        option.sctpStreamParameters = sctpStreamParameters,
        option.label = label;
        option.protocol = protocol;
        option._appData = appData;
        auto dataProducer =  transport->produceData(option);
        
        // Store the Producer into the protoo Peer data Object.
        peer->data.dataProducers[dataProducer->id()]= dataProducer;
        
        accept({ {"id", dataProducer->id()} });
        
        if (true)
        {
            if(dataProducer->label() == "chat")
            {
                // Create a server-side DataConsumer for each Peer.
                for (auto  otherPeer : this->getJoinedPeers( peer ))
                {
                    this->createDataConsumer(
                                             
                                             /*dataConsumerPeer :*/ otherPeer,
                                             /*dataProducerPeer :*/ peer,
                                             dataProducer
                                             );
                }
                
                //break;
            }else
                if(dataProducer->label() ==  "bot")
                {
                    // Pass it to the bot.
                    //                        this->_bot.handlePeerDataProducer(
                    //                            {
                    //                                dataProducerId : dataProducer->id(),
                    //                                peer
                    //                            });
                    
                    //    //break;
                }
        }
        
        //break;
    }else if(method ==  "changeDisplayName"){
        // Ensure the Peer is joined.
        if (!peer->data.joined)
            MS_THROW_lOG("Peer not yet joined");
        auto data = request["data"];
        auto displayName = data["displayName"];
        auto oldDisplayName = peer->data.displayName;
        
        // Store the display name into the custom data Object of the protoo
        // Peer.
        peer->data.displayName = displayName;
        
        // Notify other joined Peers.
        for (auto  &otherPeer : this->getJoinedPeers( peer ))
        {
            otherPeer->notifyAsync(
                                   "peerDisplayNameChanged",
                                   {
                {"peerId" , peer->getPeerId()},
                {"displayName",displayName},
                {"oldDisplayName",oldDisplayName}
            });
            
        }
        
        accept(json({}));
        
        //break;
    }else if(method ==  "getTransportStats"){
        auto data = request["data"];
        auto transportId = data["transportId"];
        auto &transport = peer->data.transports[transportId];
        
        if (!transport)
            MS_THROW_lOG("transport with id transportId=%s not found",transportId.dump().c_str());
        auto stats =  transport->getStats();
        accept(stats);
    }else if(method ==  "getProducerStats"){
        auto data = request["data"];
        auto & producerId = data["producerId"];
        auto &producer = peer->data.producers[producerId];
        
        if (!producer)
            MS_THROW_lOG("producer with id producerId=%s not found",producerId.dump().c_str());
        
        auto stats =  producer->getStats();
        
        accept(stats);
        
        //break;
    }else if(method ==  "getConsumerStats"){
        auto data = request["data"];
        auto & consumerId = data["consumerId"];
        auto  &consumer = peer->data.consumers[consumerId];
        
        if (!consumer)
            MS_THROW_lOG("consumer with id consumerId=%s not found",consumerId.dump().c_str());
        
        auto stats =  consumer->getStats();
        
        accept(stats);
        
        //break;
    }else if(method ==  "getDataProducerStats"){
        auto data = request["data"];
        auto & dataProducerId = data["dataProducerId"];
        auto &dataProducer = peer->data.dataProducers[dataProducerId];
        
        if (!dataProducer)
            MS_THROW_lOG("dataProducer with id dataProducerId=%s not found",dataProducerId.dump().c_str());
        
        auto stats =  dataProducer->getStats();
        
        accept(stats);
        
        //break;
    }else if(method ==  "getDataConsumerStats"){
        auto data = request["data"];
        auto & dataConsumerId = data["dataConsumerId"];
        auto &dataConsumer = peer->data.dataConsumers[dataConsumerId];
        
        if (!dataConsumer)
            MS_THROW_lOG("dataConsumer with id dataConsumerId=%s not found",dataConsumerId.dump().c_str());
        
        auto stats =  dataConsumer->getStats();
        
        accept(stats);
    }else if(method ==  "applyNetworkThrottle"){
        auto DefaultUplink = 1000000;
        auto DefaultDownlink = 1000000;
        auto DefaultRtt = 0;
        
        auto data = request["data"];
        auto & uplink = data["uplink"];
        auto & downlink = data["downlink"];
        auto & rtt = data["rtt"];
        auto & secret = data["secret"];
        if (!secret)// || secret != process.env.NETWORK_THROTTLE_SECRET)
        {
            reject(403, "operation NOT allowed, modda fuckaa");
            
            return;
        }
        
        //                try
        //                {
        //                     throttle.start(
        //                        {
        //                            up   : uplink || DefaultUplink,
        //                            down : downlink || DefaultDownlink,
        //                            rtt  : rtt || DefaultRtt
        //                        });
        //
        //                    MS_lOGW(
        //                        "network throttle set [uplink:%s, downlink:%s, rtt:%s]",
        //                        uplink || DefaultUplink,
        //                        downlink || DefaultDownlink,
        //                        rtt || DefaultRtt);
        //
        //                accept(json({}));
        //                }
        //                catch (const char *error)
        //                {
        //                    MS_lOGE("network throttle apply failed: %s", error);
        //
        //                    reject(500, error.toString());
        //                }
        
        //break;
    }else if(method ==  "resetNetworkThrottle"){
        auto data = request["data"];
        auto & secret = data["secret"];
        if (!secret)// || secret != process.env.NETWORK_THROTTLE_SECRET)
        {
            reject(403, "operation NOT allowed, modda fuckaa");
            
            return;
        }
        
        //                try
        //                {
        //                     throttle.stop({});
        //
        //                    MS_lOGW("network throttle stopped");
        //
        //                    accept(json({}));
        //                }
        //                catch (const char *error)
        //                {
        //                    MS_lOGE("network throttle stop failed: %s", error);
        //
        //                    reject(500, error.toString());
        //                }
        
        //break;
    }else
    {
        MS_lOGE("unknown request.method %s", method.c_str());
        
        reject(500, "unknown request.method request.method=%s");
    }
}

void Room::handleNotification(json notification){

}


oatpp::List<oatpp::Object<MessageDto>> Room::getHistory() {

  if(!m_appConfig->maxRoomHistoryMessages || *m_appConfig->maxRoomHistoryMessages == 0) {
    return nullptr;
  }

  auto result = oatpp::List<oatpp::Object<MessageDto>>::createShared();

  std::lock_guard<std::mutex> guard(m_historyLock);

  for(auto& message : m_history) {
    result->push_back(message);
  }

  return result;

}

void Room::sendMessageAsync(const oatpp::Object<MessageDto>& message) {
//  std::lock_guard<std::mutex> guard(m_peersLock);
//  for(auto& pair : m_peers) {
//    pair.second->sendMessageAsync(message);
//  }
}

void Room::pingAllPeers() {
  std::lock_guard<std::mutex> guard(m_peersLock);
  for(auto& pair : m_peers) {
    auto& peer = pair.second;
    if(!peer->sendPingAsync()) {
      peer->invalidateSocket();
      ++ m_statistics->EVENT_PEER_ZOMBIE_DROPPED;
    }
  }
}

bool Room::isEmpty() {
  return m_peers.size() == 0;
}

/**
     * Helper to get the list of joined protoo peers.
     */
    std::vector<shared_ptr<Peer> > Room::getJoinedPeers(std::shared_ptr<Peer> excludePeer)
    {
        std::vector<std::shared_ptr<Peer> > peerlist;
        for (const auto& kv : this->m_peers)
        {
            auto peerId   = kv.first;
            auto peer   = kv.second;
            if(peer->data.joined && peer != excludePeer)
            {
                 peerlist.push_back(peer);
            }
             
        }
        return peerlist;
        //return this->_protooRoom->peers
        //    .filter((peer) => peer->data.joined && peer != excludePeer);
    }

/**
     * Helper to get the list of joined protoo peers.
     */
    std::unordered_map<std::string,shared_ptr<Peer>> Room::getJoinedPeersMap(std::shared_ptr<Peer> excludePeer)
    {
       std::unordered_map<std::string,shared_ptr<Peer> > peerMap;
        for (const auto& kv : this->m_peers)
        {
            auto peerId   = kv.first;
            auto peer   = kv.second;
            if(peer->data.joined && peer != excludePeer)
            {
                peerMap.insert(kv);
            }
             
        }
        return peerMap;
        //return this->_protooRoom->peers
        //    .filter((peer) => peer->data.joined && peer != excludePeer);
    }

/**
 * Creates a mediasoup Consumer for the given mediasoup producer->
 *
 * @async
 */
void Room::createConsumer(std::shared_ptr<Peer> &consumerPeer, std::shared_ptr<Peer> &producerPeer, std::shared_ptr<Producer>  &producer)
{
    // Optimization:
    // - Create the server-side Consumer in paused mode.
    // - Tell its Peer about it and wait for its response.
    // - Upon receipt of the response, resume the server-side consumer->
    // - If video, this will mean a single key frame requested by the
    //   server-side Consumer (when resuming it).
    // - If audio (or video), it will avoid that RTP packets are received by the
    //   remote endpoint *before* the Consumer is locally created in the endpoint
    //   (and before the local SDP O/A procedure ends). If that happens (RTP
    //   packets are received before the SDP O/A is done) the PeerConnection may
    //   fail to associate the RTP stream.

    // NOTE: Don"t create the Consumer if the remote Peer cannot consume it.
    RtpCapabilities rtpCapabilities = consumerPeer->data.rtpCapabilities;
    if (
        rtpCapabilities.codecs.size() ==0  ||
        !this->m_mediasoupRouter->canConsume(
                producer->id(),
                rtpCapabilities
            )
    )
    {
        MS_lOGW("_createConsumer() | Transport canConsume = false return");
        return;
    }

    // Must take the Transport the remote Peer is using for consuming.
//        auto transport = Array.from(consumerPeer.data.transports)
//            .find((t) => t.appData.consuming);

    std::shared_ptr<Transport> transport;
    for(auto &kv : consumerPeer->data.transports)
    {
        auto &t = kv.second;
        if(t->appData()["consuming"].get<bool>() == true) {
            transport = t;
            break;
        }
    }
    // This should not happen.
    if (!transport)
    {
        MS_lOGW("_createConsumer() | Transport for consuming not found");

        return;
    }

    // Create the Consumer in paused mode.
    std::shared_ptr<Consumer> consumer;

    try
    {
          ConsumerOptions options;
          options.producerId = producer->id();
          options.rtpCapabilities = consumerPeer->data.rtpCapabilities;
          options.paused          = true;
        consumer =  transport->consume(options);
        MS_lOGD("consumerPeer->data.rtpCapabilities=%s",consumerPeer->data.rtpCapabilities.dump(4).c_str());
        json jrtpParameters=consumer->rtpParameters();
        MS_lOGD("consumer->rtpParameters()=%s",jrtpParameters.dump(4).c_str());
    }
    catch (const char *error)
    {
        MS_lOGW("_createConsumer() | transport->consume():%s", error);

        return;
    }

    // Store the Consumer into the protoo consumerPeer data Object.
    consumerPeer->data.consumers[consumer->id()] =  consumer;

    // Set Consumer events.
    consumer->on("transportclose",[consumerPeer, consumer]( )
    {
        // Remove from its map.
        consumerPeer->data.consumers.erase(consumer->id());
    });

    consumer->on("producerclose",[consumerPeer, consumer](  )
    {
        
        MS_lOGD("producerclose  producerclose  producerclose()");
        
        // Remove from its map.
        consumerPeer->data.consumers.erase(consumer->id());

        consumerPeer->notifyAsync("consumerClosed", { {"consumerId", consumer->id()} });
    });

    consumer->on("producerpause",[consumerPeer, consumer]()
    {
        consumerPeer->notifyAsync("consumerPaused", { {"consumerId", consumer->id()} });
    });

    consumer->on("producerresume",[consumerPeer, consumer](  )
    {
        consumerPeer->notifyAsync("consumerResumed", { {"consumerId", consumer->id()} });
    });

    consumer->on("score", [consumerPeer, consumer](int score ) //(score) =>
    {
       
        // MS_lOGD(
        //     "consumer "score" event [consumerId:%s, score:%o]",
        //     consumer->id(), score);
        
        consumerPeer->notifyAsync("consumerScore", {
          {"consumerId", consumer->id()},
          {"score",score}
          });
        

    });

    consumer->on("layerschange",[consumerPeer, consumer]( json layers ) // (layers) =>
    {
  
        consumerPeer->notifyAsync(
            "consumerLayersChanged",
            {
                {"consumerId"    , consumer->id()},
                {"spatialLayer"  , layers["spatialLayer"]},
                {"temporalLayer" , layers["temporalLayer"]}
            });
    });

    // NOTE: For testing.
    //  consumer->enableTraceEvent([ "rtp", "keyframe", "nack", "pli", "fir" ]);
    //  consumer->enableTraceEvent([ "pli", "fir" ]);
    //  consumer->enableTraceEvent([ "keyframe" ]);

    consumer->on("trace",[consumer]( ConsumerTraceEventData & trace ) // (trace) =>
    {
        
        MS_lOGD(
            "consumer trace event [producerId:%s, trace.type:%s, trace]",
            consumer->id().c_str(), trace.type.c_str());
    });

    // Send a protoo request to the remote Peer with Consumer parameters.
    //if(!consumerPeer->isHasRequest) {
        // consumer->resume();
        //return;
    //}
    try
    {
        MS_lOGD("===========newConsumer==============");
         consumerPeer->requestAsync(
            "newConsumer",
            {
             {"peerId"     , producerPeer->getPeerId()},
             {"producerId"         , producer->id()},
             {"id"         , consumer->id()},
             {"kind"         , consumer->kind()},
             {"rtpParameters"         , consumer->rtpParameters()},
             {"type"         , consumer->type()},
             {"appData", producer->appData()},
             {"producerPaused", consumer->producerPaused()}
            });


              

        // Now that we got the positive response from the remote endpoint, resume
        // the Consumer so the remote endpoint will receive the a first RTP packet
        // of this new stream once its PeerConnection is already ready to process
        // and associate it.

        //if(consumerPeer->isHasNotify) {
            // consumer->resume();
            //return;
        //}
       
//        consumerPeer->notifyAsync(
//            "consumerScore",
//            {
//                {"consumerId" , consumer->id()},
//                {"score"      , consumer->score()}
//            });

         //consumer->resume();
    }
    catch (const char *error)
    {
        MS_lOGW("_createConsumer() | failed:%s", error);
    }
    return;
}


/**
 * Creates a mediasoup DataConsumer for the given mediasoup dataProducer->
 *
 * @async
 */
void Room::createDataConsumer(
    //{
                         std::shared_ptr<Peer> &dataConsumerPeer,
                         std::shared_ptr<Peer> dataProducerPeer, // This is null for the bot dataProducer->
                         std::shared_ptr<DataProducer> dataProducer
    //}
                         )
{
    // NOTE: Don"t create the DataConsumer if the remote Peer cannot consume it.
    if (!dataConsumerPeer->data.sctpCapabilities.dump().empty())
        return;

    // Must take the Transport the remote Peer is using for consuming.
//        auto transport = Array.from(dataConsumerPeer.data.transports)
//            .find((t) => t.appData.consuming);
    std::shared_ptr<Transport> transport;
    for(auto &kv : dataConsumerPeer->data.transports)
    {
        auto t = kv.second;
        if(t->appData()["consuming"].get<bool>() == true) {
            transport = t;
            break;
        }
    }
    // This should not happen.
    if (!transport)
    {
        MS_lOGW("_createDataConsumer() | Transport for consuming not found");

        return;
    }

    // Create the Dataconsumer->
    std::shared_ptr<DataConsumer> dataConsumer;

    try
    {
          DataConsumerOptions options;
          options.dataProducerId = dataProducer->id();
        dataConsumer =  transport->consumeData(options);
    }
    catch (const char * error)
    {
        MS_lOGW("_createDataConsumer() | transport->consumeData():%s", error);

        return;
    }

    // Store the DataConsumer into the protoo dataConsumerPeer data Object.
    dataConsumerPeer->data.dataConsumers[dataConsumer->id()]= dataConsumer;

    // Set DataConsumer events.
    dataConsumer->on("transportclose",[dataConsumerPeer, dataConsumer](  )
    {
        // Remove from its map.
        dataConsumerPeer->data.dataConsumers.erase(dataConsumer->id());
    });

    dataConsumer->on("dataproducerclose",[dataConsumerPeer, dataConsumer]( )
    {
        // Remove from its map.
        dataConsumerPeer->data.dataConsumers.erase(dataConsumer->id());

        dataConsumerPeer->notifyAsync(
                                "dataConsumerClosed", { {"dataConsumerId", dataConsumer->id()} });
    });

    // Send a protoo request to the remote Peer with Consumer parameters.
    try
    {
         dataConsumerPeer->requestAsync(
            "newDataConsumer",
            {
                // This is null for bot dataProducer->
                {"peerId"            , dataProducerPeer->getPeerId() },
                {"dataProducerId"       ,  dataProducer->id() },
                {"id"                   ,  dataConsumer->id() },
                {"sctpStreamParameters" ,  dataConsumer->sctpStreamParameters() },
                {"label"                ,  dataConsumer->label() },
                {"protocol"             ,  dataConsumer->protocol() },
                {"appData"             ,  dataProducer->appData() }
            });
    }
    catch (...)
    {
        MS_lOGW("_createDataConsumer() | failed:");
    }
}
