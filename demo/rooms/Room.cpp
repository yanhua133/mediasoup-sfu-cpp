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
  std::lock_guard<std::mutex> guard(m_peerByIdLock);
  m_peerById[peer->getPeerId()] = peer;
}

void Room::welcomePeer(const std::shared_ptr<Peer>& peer) {

  /* Inform all that peer have joined the room */

  auto joinedMessage = MessageDto::createShared();
  joinedMessage->code = MessageCodes::CODE_PEER_JOINED;
  joinedMessage->peerId = oatpp::String(peer->getPeerId().c_str());
  joinedMessage->peerName = peer->getNickname();
  joinedMessage->message = peer->getNickname() + " - joined room";

  addHistoryMessage(joinedMessage);
  sendMessageAsync(joinedMessage);

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

  auto message = MessageDto::createShared();
  message->code = MessageCodes::CODE_PEER_LEFT;
  //message->peerId = peer->getPeerId();
  message->message = peer->getNickname() + " - left room";

  addHistoryMessage(message);
  sendMessageAsync(message);

}

std::shared_ptr<Peer> Room::getPeerById(std::string peerId) {
  std::lock_guard<std::mutex> guard(m_peerByIdLock);
  auto it = m_peerById.find(peerId);
  if(it != m_peerById.end()) {
    return it->second;
  }
  return nullptr;
}

void Room::removePeerById(std::string peerId) {

  std::lock_guard<std::mutex> guard(m_peerByIdLock);
  auto peer = m_peerById.find(peerId);

  if(peer != m_peerById.end()) {
    m_peerById.erase(peerId);

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

void Room::handleRequest(Peer* peer, json &request, std::function<void(json data)> const & accept, std::function<void(int errorCode, std::string errorReason)> const & reject){
    std::string method = request["method"];
            //switch (request.method)
            MS_lOGD("_handleProtooRequest request data = %s",request["data"].dump().c_str());
            if(true)
            {
                if(method == "getRouterRtpCapabilities")
                {
                    json rtpCapabilities = this->m_mediasoupRouter->rtpCapabilities();
                    MS_lOGE("_handleProtooRequest getRouterRtpCapabilities = %s",rtpCapabilities.dump(4).c_str());
                    accept(rtpCapabilities);

                    //break;
                }else
                if(method ==  "join")
                {
                    // Ensure the Peer is not already joined.
                    if (peer->data.joined)
                    {
                        MS_THROW_lOG("Peer already joined");
                       // accept(json({}));
                       // return;
                    }
                    auto data = request["data"];
                    auto displayName = data["displayName"];
                    auto device = data["device"];
                    auto rtpCapabilities = data["rtpCapabilities"];
                    auto sctpCapabilities = data["sctpCapabilities"];
                    MS_lOGD("join rtpCapabilities=%s",rtpCapabilities.dump(4).c_str());

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
                    std::unordered_map<std::string, shared_ptr<Peer>> joinPeers = this->_getJoinedPeersMap();
                    joinedPeers.insert(joinPeers.begin(), joinPeers.end());
                    //joinedPeers.insert(this->_broadcasters.begin(), this->_broadcasters.end());
                    //joinedPeers.insert(this->_bridges.begin(), this->_bridges.end());

                            // Reply now the request with the list of joined peers (all but the new one).
                    auto it = joinedPeers.find(peer->id);
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
                            {"id"          , joinedPeer->id},
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
                            this->_createConsumer(
                                
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

                            this->_createDataConsumer(
                                    /*dataConsumerPeer :*/ peer,
                                    /*dataProducerPeer :*/ joinedPeer,
                                    dataProducer
                                );
                        }
                    }

                    // Create DataConsumers for bot dataProducer->
                    this->_createDataConsumer(
                        
                            /*dataConsumerPeer :*/ peer,
                            /*dataProducerPeer :*/ nullptr,
                            /*dataProducer     :*/ nullptr//this->_bot.dataProducer
                        );

                    // Notify the new Peer to all other Peers.
                    for (auto  &otherPeer : this->_getJoinedPeers( peer ))
                    {
                        if(!otherPeer)
                        {
                            continue;
                        }
                        otherPeer->notify(
                            "newPeer",
                            {
                                {"id"          , peer->id},
                                {"displayName" , peer->data.displayName},
                                {"device"      , peer->data.device}
                            });
                            
                    }

                    //break;
                }else
                if(method ==  "createWebRtcTransport")
                {
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
                   
                    json jwebRtcTransportOptions = config.mediasoup.webRtcTransportOptions;
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

                    auto transport =  this->_mediasoupRouter->createWebRtcTransport(webRtcTransportOptions);

                    transport->on("sctpstatechange", [&]( std::string sctpState ) //(sctpState) =>
                    {
                        MS_lOGD("WebRtcTransport sctpstatechange event [sctpState:%s]", sctpState.c_str());
                    });

                    transport->on("dtlsstatechange", [&]( std::string dtlsState ) //(dtlsState) =>
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

                    transport->on("trace", [&]( TransportTraceEventData & trace ) //(trace) =>
                    {
                        
                        MS_lOGD(
                            "transport trace event [transportId:%s, trace.type:%s, trace]",
                            transport->id().c_str(), trace.type.c_str());

                        if (trace.type == "bwe" && trace.direction == "out")
                        {
                            peer->notify(
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
                    auto &maxIncomingBitrate = config.mediasoup.webRtcTransportOptions["maxIncomingBitrate"];
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
                }else
                if(method ==  "connectWebRtcTransport")
                {
                    auto data = request["data"];
                    auto transportId = data["transportId"];
                    auto dtlsParameters = data["dtlsParameters"];
                    auto &transport = peer->data.transports[transportId];

                    if (!transport)
                        MS_THROW_lOG("transport with id transportId=%s not found",transportId.dump().c_str());

                     json newdata= json::object();
                     newdata["dtlsParameters"] = dtlsParameters;
                     MS_lOGD("connectWebRtcTransport dtlsParameters=%s",newdata.dump(4).c_str());
                     transport->connect(newdata);

                    accept(json({}));

                    //break;
                }else
                if(method ==  "restartIce")
                {
                    auto data = request["data"];
                    auto & transportId = data["transportId"];
                    auto &transport = peer->data.transports[transportId];

                    if (!transport)
                    {
                        MS_THROW_lOG("transport with id transportId=%s not found",transportId.dump().c_str());
                        accept(json({}));
                        return;
                    }

                    auto iceParameters =  transport->restartIce();

                    accept(iceParameters);

                    
                }else
                if(method ==  "produce")
                {
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
                    appData["peerId"] = peer->id;
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
                    producer->on("score", [&]( int score) //(score) =>
                    {
                     
                        // MS_lOGD(
                        //     "producer "score" event [producerId:%s, score:%o]",
                        //     producer->id(), score);

                        peer->notify("producerScore", { {"producerId", producer->id()},{ "score",score} });
                            
                    });

                    producer->on("videoorientationchange",[&]( int videoOrientation ) // (videoOrientation) =>
                    {
                       
                        MS_lOGD(
                            "producer videoorientationchange event [producerId:%s, videoOrientation:%o]",
                            producer->id().c_str(), videoOrientation);
                    });

                    // NOTE: For testing.
                    //  producer->enableTraceEvent([ "rtp", "keyframe", "nack", "pli", "fir" ]);
                    //  producer->enableTraceEvent([ "pli", "fir" ]);
                    //  producer->enableTraceEvent([ "keyframe" ]);

                    producer->on("trace", [&](TransportTraceEventData & trace) //(trace) =>
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
                      std::unordered_map<std::string, shared_ptr<Peer>> joinPeers = this->_getJoinedPeersMap(peer);
                      joinedPeers.insert(joinPeers.begin(), joinPeers.end());
                      joinedPeers.insert(this->_broadcasters.begin(), this->_broadcasters.end());

                    // Optimization: Create a server-side Consumer for each Peer.
                    if(kind == "audio") {
                        auto tt=0;
                        tt=1;

                    } else {

                    }

                    for (auto  &kv : joinedPeers)
                    {
                        auto otherPeer = kv.second;
                        if(otherPeer->id == peer->id) {
                            continue;
                        }
                        this->_createConsumer(
                            
                                /*consumerPeer :*/ otherPeer,
                                /*producerPeer :*/ peer,
                                producer
                            );

                    }
                    // 这块需要单独处理，因为需要采集全音频和视频consumer，但是js的设置好像有点延迟，音频设置consumer后但是外层变量没有显示，所以只能先放到这个函数里实现
                    for (auto  &kv : this->_bridges)
                    {
                        auto otherPeer = kv.second;
                        if(otherPeer->id == peer->id) {
                            continue;
                        }
                        this->_createBridgeConsumer(
                            
                                /*consumerPeer :*/ otherPeer,
                                /*producerPeer :*/ peer,
                                producer
                            );

                    }
                    if( paused == true) {
                         producer->pause();
                    }
                    // Add into the audioLevelObserver.
                    if (producer->kind() == "audio")
                    {
                        this->_audioLevelObserver->addProducer(producer->id());
                            
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
                    if(this->dvr) {
                      if(hasVideo && hasAudio) {
                        //mcuPublish(this->_mediasoupRouter, peer);
                        //startRecord(this->_mediasoupRouter, peer, producer,this->_roomid());
                      }
                    }
        
                    //break;
                }else
                if(method ==  "closeProducer")
                {
                    // Ensure the Peer is joined.
                    if (!peer->data.joined)
                        MS_THROW_lOG("Peer not yet joined");
                    auto data = request["data"];
                    auto & producerId = data["producerId"];
                    auto & producer = peer->data.producers[producerId];

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
                    if(this->dvr) {
                      if(hasAudio) {
                          transfmt.stopRecord(peer->id);
                      }
                     
                    }
                    //break;
                }else
                if(method ==  "pauseProducer")
                {
                    // Ensure the Peer is joined.
                    if (!peer->data.joined)
                        MS_THROW_lOG("Peer not yet joined");
                    auto data = request["data"];
                    auto & producerId = data["producerId"];
                    auto &producer = peer->data.producers[producerId];

                    if (!producer)
                        MS_THROW_lOG("producer with id producerId=%s not found",producerId.dump().c_str());

                     producer->pause();

                    accept(json({}));

                    //break;
                }else
                if(method ==  "resumeProducer")
                {
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

                    //break;
                }else
                if(method ==  "pauseConsumer")
                {
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
                }else
                if(method ==  "resumeConsumer")
                {
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
                }else
                if(method ==  "setConsumerPreferredLayers")
                {
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
                }else
                if(method ==  "setConsumerPriority")
                {
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
                }else
                if(method ==  "requestConsumerKeyFrame")
                {
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
                }else
                if(method ==  "produceData")
                {
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
                            for (auto  otherPeer : this->_getJoinedPeers( peer ))
                            {
                                this->_createDataConsumer(
                                    
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
                }else
                if(method ==  "changeDisplayName")
                {
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
                    for (auto  &otherPeer : this->_getJoinedPeers( peer ))
                    {
                        otherPeer->notify(
                            "peerDisplayNameChanged",
                            {
                            {"peerId" , peer->id},
                            {"displayName",displayName},
                            {"oldDisplayName",oldDisplayName}
                            });
                            
                    }

                    accept(json({}));

                    //break;
                }else
                if(method ==  "getTransportStats")
                {
                    auto data = request["data"];
                    auto transportId = data["transportId"];
                    auto &transport = peer->data.transports[transportId];

                    if (!transport)
                        MS_THROW_lOG("transport with id transportId=%s not found",transportId.dump().c_str());

                    auto stats =  transport->getStats();

                    accept(stats);

                    //break;
                }else
                if(method ==  "getProducerStats")
                {
                    auto data = request["data"];
                    auto & producerId = data["producerId"];
                    auto &producer = peer->data.producers[producerId];

                    if (!producer)
                        MS_THROW_lOG("producer with id producerId=%s not found",producerId.dump().c_str());

                    auto stats =  producer->getStats();

                    accept(stats);

                    //break;
                }else
                if(method ==  "getConsumerStats")
                {
                    auto data = request["data"];
                    auto & consumerId = data["consumerId"];
                    auto  &consumer = peer->data.consumers[consumerId];

                    if (!consumer)
                        MS_THROW_lOG("consumer with id consumerId=%s not found",consumerId.dump().c_str());

                    auto stats =  consumer->getStats();

                    accept(stats);

                    //break;
                }else
                if(method ==  "getDataProducerStats")
                {
                    auto data = request["data"];
                    auto & dataProducerId = data["dataProducerId"];
                    auto &dataProducer = peer->data.dataProducers[dataProducerId];

                    if (!dataProducer)
                        MS_THROW_lOG("dataProducer with id dataProducerId=%s not found",dataProducerId.dump().c_str());

                    auto stats =  dataProducer->getStats();

                    accept(stats);

                    //break;
                }else
                if(method ==  "getDataConsumerStats")
                {
                    auto data = request["data"];
                    auto & dataConsumerId = data["dataConsumerId"];
                    auto &dataConsumer = peer->data.dataConsumers[dataConsumerId];

                    if (!dataConsumer)
                        MS_THROW_lOG("dataConsumer with id dataConsumerId=%s not found",dataConsumerId.dump().c_str());

                    auto stats =  dataConsumer->getStats();

                    accept(stats);

                    //break;
                }else
                if(method ==  "applyNetworkThrottle")
                {
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
                }else
                if(method ==  "resetNetworkThrottle")
                {
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
//  std::lock_guard<std::mutex> guard(m_peerByIdLock);
//  for(auto& pair : m_peerById) {
//    pair.second->sendMessageAsync(message);
//  }
}

void Room::pingAllPeers() {
  std::lock_guard<std::mutex> guard(m_peerByIdLock);
  for(auto& pair : m_peerById) {
    auto& peer = pair.second;
    if(!peer->sendPingAsync()) {
      peer->invalidateSocket();
      ++ m_statistics->EVENT_PEER_ZOMBIE_DROPPED;
    }
  }
}

bool Room::isEmpty() {
  return m_peerById.size() == 0;
}
