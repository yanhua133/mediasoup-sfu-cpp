#include "Server.hpp"
// #include <webservice/file_request_handler.hpp>
// #include <webservice/ws_service.hpp>
// #include <webservice/ws_session.hpp>
// #include <webservice/server.hpp>
#include "Log.hpp"
#include <iostream>
#include <csignal>
#include "RtpParameters.hpp"
#include "sdp/SdpOffer.hpp"

// struct mirror_ws_service: webservice::ws_service{
//     void on_open(webservice::ws_identifier identifier)override{
//         std::cout << "open session " << identifier << "\n";
//     }

//     void on_close(webservice::ws_identifier identifier)override{
//         std::cout << identifier << " closed\n";
//     }

//     void on_text(
//         webservice::ws_identifier identifier,
//         std::string&& text
//     )override{
//         std::cout << identifier << " received text message: " << text << "\n";

//         // Send received text to all WebSocket sessions
//         send_text(text);
//     }

//     void on_binary(
//         webservice::ws_identifier identifier,
//         std::vector< std::uint8_t >&& data
//     )override{
//         std::cout << identifier << " received binary message\n";

//         // Send received data to all WebSocket sessions
//         send_binary(data);
//     }
// };


// void on_interrupt(int signum){
//     std::signal(signum, SIG_DFL);
//     std::cout << "Signal: " << signum << "\n";
//    // server->shutdown();
//     std::cout << "Signal ready\n";
// }


// void print_help(char const* exec_name){
//     std::cerr << "Usage: " << exec_name
//         << " <address> <port> <doc_root> <thread_count>\n"
//         << "Example:\n"
//         << "    " << exec_name << " 0.0.0.0 8080 http_root_directory 1\n";
// }

SfuServer::SfuServer()
{
    
}
SfuServer::~SfuServer()
{
  // if(rawWebsockServer != nullptr) 
  // {
  //     delete rawWebsockServer;
  //     rawWebsockServer = nullptr;
  // }
  

  if(mediasoup != nullptr) {
      mediasoup->Destroy();
      mediasoup::DestroyMediasoup(mediasoup);
  }

}

void SfuServer::setConfig(std::shared_ptr<Config> pConfig) {
    this->m_pConfig = pConfig;
}

int SfuServer::init()
{
    initMediasoup();
    createRawWebSocket();
    runRawWebsocket();
   createProtooWebSocket();
}

void SfuServer::run()
{
    runProtooWebSocketServer();

    //testProtoo();
}
int SfuServer::createRawWebSocket()
{
//    try{
//        auto address = boost::asio::ip::make_address(this->listenip);
//        auto port = this->listenport;
//        rawWebsockServer = new webservice::server(
//                       nullptr,
//                       std::make_unique< mirror_ws_service >(),
//                       nullptr, // ignore errors and exceptions
//                       address, port, this->thread_count);
//        return 0;
//    }catch(std::exception const& e){
//        std::cerr << "Exception: " << e.what() << "\n";
//        return 1;
//    }catch(...){
//        std::cerr << "Unknown exception\n";
//        return 1;
//    }
    return 0;
}
int SfuServer::createProtooWebSocket()
{
    //protooWebsockServer = std::make_shared<WebSocketServer>("0.0.0.0",8001,"");//new WebSocketServer("0.0.0.0",8001,"");
    //protooWebsockServer->runWebSocketServer();
   
}
void SfuServer::processRawSdpMessage(std::string message)
{
//    auto msg = json::parse(message); //根据请求过来的数据来更新。
//    MS_lOGD("raw websocket recv message %s", msg.dump().c_str());
//    auto recipientClientId = msg["RecipientClientId"];
//    auto messagePayload = msg["MessagePayload"];
//    auto sdpAnswer = getStringFromBase64(messagePayload);
//    MS_lOGD("messagePayload=%s",sdpAnswer.dump().c_str());
//    auto roomId = msg["CorrelationId"].get<std::string>();
//    oatpp::String oatppRoomId = roomId.c_str();
//    auto room = m_rooms[oatppRoomId];
//    std::string bridgeId = msg["RecipientClientId"];
//    auto transportId = room->getBridgeTransportId(bridgeId);
//
//
//
//    try
//    {
//        auto dtlsParameters = mediasoupclient::Sdp::Offer::getMediasoupDtlsParameters(sdpAnswer["sdp"]);
//        json jdtlsParameters = dtlsParameters;
//        MS_lOGD("dtlsParameters=%s",jdtlsParameters.dump().c_str());
//        room->connectBridgeTransport(
//
//                bridgeId,
//                transportId,
//                dtlsParameters
//        );
//
//
//
//        std::string kind = "audio";
//
//        auto rtpParameters = mediasoupclient::Sdp::Offer::getMediasoupRtpParameters(sdpAnswer["sdp"],kind,room->getLocalSdp());
//        room->createBridgeProducer(
//
//                bridgeId,
//                transportId,
//                kind,
//                rtpParameters
//            );
//
//         kind = "video";
//         rtpParameters = mediasoupclient::Sdp::Offer::getMediasoupRtpParameters(sdpAnswer["sdp"],kind,room->getLocalSdp());
//        room->createBridgeProducer(
//
//                bridgeId,
//                transportId,
//                kind,
//                rtpParameters
//            );
//    }
//    catch (...)
//    {
//        MS_lOGE("error");
//    }
//    if(msg["action"] == "SDP_OFFER") {
//        auto webRtcTransport = room->getBridgeTransport(bridgeId);
//        auto producers = room->getProducersFromBridge(bridgeId);
//        auto producerMedias=mediasoupclient::Sdp::Offer::getMediasoupProducerMedias(sdpAnswer["sdp"]);
//        auto sdpAnswer = mediasoupclient::Sdp::Offer::createWebrtcSdpAnswer(webRtcTransport,producers,producerMedias);
//        json payload = {
//            {"type","offer"},
//            {"sdp",sdpAnswer}
//        };
//        Base64 base;
//        auto msgpayload = base.Encode((const unsigned char*)payload.dump().c_str(),payload.dump().length());
//        //auto b = new Buffer.from(JSON.stringify(payload));
//       // auto msgpayload = b.toString("base64");
//        json jsonmsg = {
//            {"action","SDP_OFFER"},
//            {"RecipientClientId","peerId"},
//            {"MessagePayload",msgpayload}
//        };
//        //ws.send(JSON.stringify(jsonmsg));
//    }
}
void SfuServer::runRawWebsocket()
{

}
void SfuServer::runRawWebsockServer()
{
    //std::signal(SIGINT, &on_interrupt);
    //rawWebsockServer->block();
}
/**
 * Get next mediasoup Worker.
 */
std::shared_ptr<mediasoup::IWorker> SfuServer::getMediasoupWorker()
{
	auto worker = workers[nextMediasoupWorkerIdx];

	if (++nextMediasoupWorkerIdx == workers.size())
		nextMediasoupWorkerIdx = 0;

	return worker;
}
void SfuServer::processHttpRequest(std::string &path,std::string & roomId,std::shared_ptr<Room> room,json &params,json &query,json & body,json &respdata)
{
  	/**
	 * For every API request, verify that the roomId in the path matches and
	 * existing room->
	 */
/*  
	expressApp.param(
		"roomId", async (req, res, next, roomId) =>
		{
			//just create the room with roomId
			getOrCreateRoom({roomId});
			// The room must exist for all API requests.
			if (!rooms.has(roomId))
			{
				auto error = new Error("room with id "${roomId}" not found");

				error.status = 404;
				throw error;
			}

			room = rooms.get(roomId);

			next();
		});
*/
  //MS_lOGD("initBridgeSignalChannel(expressApp)");
  //initBridgeSignalChannel(null,rooms);
	/**
	 * API GET resource that returns the mediasoup Router RTP capabilities of
	 * the room->
	 */
	if(
		("/rooms/"+roomId ) ==  path )
		{
			auto data = room->getRouterRtpCapabilities();

			respdata = data ;
    }else

	/**
	 * POST API to create a Broadcaster.
	 */
	if(
		("/rooms/"+roomId+"/broadcasters") ==  path )
		{
			auto id = body["id"];
			auto displayName = body["displayName"];
			auto device = body["device"];
			auto rtpCapabilities = body["rtpCapabilities"];
      MS_lOGD("[LiveRoom] get req body=${JSON.stringify(body)}");
			try
			{
                RtpCapabilities cap = rtpCapabilities;
				auto data = room->createBroadcaster(
					
						id,
						displayName,
						device,
                        cap
					);

				//respdata = data ;
			}
			catch (...)
			{
				
			}
    }else

	/**
	 * DELETE API to delete a Broadcaster.
	 */
	if(
		("/rooms/"+roomId+"/broadcasters/:broadcasterId") ==  path )
		{
			auto broadcasterId = params["broadcasterId"];

			//room->deleteBroadcaster( broadcasterId );

			//res.status(200).send("broadcaster deleted");
    }else

	/**
	 * POST API to create a mediasoup Transport associated to a Broadcaster.
	 * It can be a PlainTransport or a WebRtcTransport depending on the
	 * type parameters in the body. There are also additional parameters for
	 * PlainTransport.
	 */
	if(
		("/rooms/"+roomId+"/broadcasters/:broadcasterId/transports") ==  path )
		{
			auto broadcasterId = params["broadcasterId"];
			auto type = body["type"];
              auto rtcpMux = body["rtcpMux"];
              auto comedia = body["comedia"];
              auto sctpCapabilities = body["sctpCapabilities"];

			try
			{
                SctpCapabilities sctp = sctpCapabilities;
//				auto data = room->createBroadcasterTransport(
//
//						broadcasterId,
//						type,
//						rtcpMux,
//						comedia,
//                        sctp
//					);
//
//				respdata = data ;
			}
			catch (...)
			{
				
			}
		}else

	/**
	 * POST API to connect a Transport belonging to a Broadcaster. Not needed
	 * for PlainTransport if it was created with comedia option set to true.
	 */
	if(
		("/rooms/"+roomId+"/broadcasters/:broadcasterId/transports/:transportId/connect")  ==  path )
		{
              auto broadcasterId = params["broadcasterId"];
              auto transportId = params["transportId"];
            DtlsParameters dtlsParameters  = body["dtlsParameters"];

			try
			{
                
				room->connectBroadcasterTransport(
					
						broadcasterId,
						transportId,
						dtlsParameters
					);

				//respdata = data ;
			}
			catch (...)
			{
				
			}
		}else

	/**
	 * POST API to create a mediasoup Producer associated to a Broadcaster.
	 * The exact Transport in which the Producer must be created is signaled in
	 * the URL path. Body parameters include kind and rtpParameters of the
	 * Producer.
	 */
	if(
		("/rooms/"+roomId+"/broadcasters/:broadcasterId/transports/:transportId/producers")  ==  path )
		{
			auto broadcasterId = params["broadcasterId"];
          auto transportId = params["transportId"];
          auto kind = body["kind"];
            RtpParameters rtpParameters = body["rtpParameters"];
			//auto { kind, rtpParameters } = body;

			try
			{
				auto data = room->createBroadcasterProducer(
					
						broadcasterId,
						transportId,
						kind,
						rtpParameters
					);

				respdata = data ;
			}
			catch (...)
			{
				
			}
		}else

	/**
	 * POST API to create a mediasoup Consumer associated to a Broadcaster.
	 * The exact Transport in which the Consumer must be created is signaled in
	 * the URL path. Query parameters must include the desired producerId to
	 * consume.
	 */
	if(
		("/rooms/"+roomId+"/broadcasters/:broadcasterId/transports/:transportId/consume") ==  path )
		{
		  auto broadcasterId = params["broadcasterId"];
          auto transportId = params["transportId"];
          auto producerId = query["producerId"];
		//	auto { producerId } = req.query;

			try
			{
				auto data = room->createBroadcasterConsumer(
					
						broadcasterId,
						transportId,
						producerId
					);

				respdata = data ;
			}
			catch (...)
			{
				
			}
		}else

	/**
	 * POST API to create a mediasoup DataConsumer associated to a Broadcaster.
	 * The exact Transport in which the DataConsumer must be created is signaled in
	 * the URL path. Query body must include the desired producerId to
	 * consume.
	 */
	if(
		("/rooms/"+roomId+"/broadcasters/:broadcasterId/transports/:transportId/consume/data") ==  path )
		{
			auto broadcasterId = params["broadcasterId"];
            auto transportId = params["transportId"];
			auto dataProducerId = body["dataProducerId"];

			try
			{
				auto data = room->createBroadcasterDataConsumer(
					
						broadcasterId,
						transportId,
						dataProducerId
					);

				respdata = data ;
			}
			catch (...)
			{
				
			}
		}else
	
	/**
	 * POST API to create a mediasoup DataProducer associated to a Broadcaster.
	 * The exact Transport in which the DataProducer must be created is signaled in
	 */
	if(
		("/rooms/"+roomId+"/broadcasters/:broadcasterId/transports/:transportId/produce/data") ==  path )
		{
			auto broadcasterId = params["broadcasterId"];
              auto transportId = params["transportId"];
              auto label = body["label"];
              auto protocol = body["protocol"];
              SctpStreamParameters sctpStreamParameters = body["sctpStreamParameters"];
              auto appData = body["appData"];

			try
			{
				auto data = room->createBroadcasterDataProducer(
					
						broadcasterId,
						transportId,
						label,
						protocol,
						sctpStreamParameters,
						appData
					);

				respdata = data ;
			}
			catch (...)
			{
				
			}
		}else



	if(
		"/describeSignalingChannel" ==  path )
		{
			/*
			* ScaryTestChannel,
		Master,
		us-west-2,
		arn:aws:kinesisvideo:us-west-2:848161169354:channel/ScaryTestChannel/1606901298484,
		https://r-fd57ec7b.kinesisvideo.us-west-2.amazonaws.com,
		wss://m-75621fd6.kinesisvideo.us-west-2.amazonaws.com,1607436362
			* */
			auto roomId= body["CorrelationId"].get<std::string>() ;
			auto bridgeId = body["RecipientClientId"];
			auto MessagePayload = body["MessagePayload"];
			MS_lOGD("describeSignalingChannel recv data=%s",body.dump().c_str());
			json ChannelInfo1 = {
                    { "ChannelInfo" , {
                            {"ChannelARN" , "arn:aws:kinesisvideo:us-west-2:848161169354:channel/1234569/1606901298484"},
                            {"ChannelName" , roomId},
                            {"Version" , "v0"},
                            {"ChannelStatus" , "CREATING"},
                            {"ChannelType" , "SINGLE_MASTER"},
                            {"CreationTime" , ""},
                        }
                    }
			};
			//just create the room with roomId
            oatpp::String oatppRoomId = roomId.c_str();
			getOrCreateRoom(oatppRoomId);
			// The room must exist for all API requests.
			if (!m_rooms[oatppRoomId])
			{
				//auto error = new Error("room with id roomId not found");

				//error.status = 404;
				//throw error;
			}

            room = m_rooms[oatppRoomId];

			try
			{

				auto sdpOffer = getStringFromBase64(MessagePayload);
				MS_lOGD("messagePayload=%s",sdpOffer.dump().c_str());
				auto room = m_rooms[oatppRoomId];
                auto remoteCaps = mediasoupclient::Sdp::Offer::getMediasoupRtpCapabilities(sdpOffer["sdp"],room->getLocalSdp());
                json device ={
                    {"name",bridgeId},
                    {"rtpCapabilities",remoteCaps }
                };
                std::string displayName = bridgeId;
                room->createBridge(bridgeId,displayName,device,remoteCaps);
                SctpCapabilities sctp;
				auto data = room->createBridgeTransport(

						/*bridgeId:*/bridgeId,
						/*type:*/"webrtc",
						/*rtcpMux:*/false,
						/*comedia:*/true,
                        sctp
					);
				MS_lOGD("createBridgeTransport transportId=%s",data["id"].dump().c_str());


			}
			catch (...)
			{
				MS_lOGE("error");
			}

			//res.status(200).json(ChannelInfo1);
		}else
	if(
		"/getSignalingChannelEndpoint" ==  path )
		{
			MS_lOGD("getSignalingChannelEndpoint recv  data=",body.dump().c_str());
			json ResourceEndpointList = {
                "ResourceEndpointList" , {
					{
						{"Protocol", "http"},
						//{"ResourceEndpoint", "http://127.0.0.1:"+config.http.listenPort},
						{"Version", "v0"},
						{"ChannelStatus","CREATING"},
						{"ChannelType", "SINGLE_MASTER"},
						{"CreationTime", ""},
					},
					{
						{"Protocol", "ws"},
					//	{"ResourceEndpoint", "ws://127.0.0.1:"+config.http.listenPort)+1)+"/"},
						{"Version", "v0"},
						{"ChannelStatus", "CREATING"},
						{"ChannelType", "SINGLE_MASTER"},
						{"CreationTime", ""},
					}
                }
			};
			//res.status(200).json(ResourceEndpointList);
		}else
	if(
		"/v1/get-ice-server-config" ==  path )
		{
			/*
			* {"IceServerList":[{"Password":"H8TZ5pAjOkalNVQJ5UgQt3Godj3URm/3lsfcJYLMwOs=","Ttl":300,"Uris":["turn:54-188-75-37.t-67e8ec01.kinesisvideo.us-west-2.amazonaws.com:443?transport=udp","turns:54-188-75-37.t-67e8ec01.kinesisvideo.us-west-2.amazonaws.com:443?transport=udp","turns:54-188-75-37.t-67e8ec01.kinesisvideo.us-west-2.amazonaws.com:443?transport=tcp"],"Username":"1610088702:djE6YXJuOmF3czpraW5lc2lzdmlkZW86dXMtd2VzdC0yOjg0ODE2MTE2OTM1NDpjaGFubmVsL1NjYXJ5VGVzdENoYW5uZWwvMTYwNjkwMTI5ODQ4NA=="},{"Password":"7Kw5G4K9SpLYEvibvZiTg48uoa5vJ7X3I/MxCkmBqAs=","Ttl":300,"Uris":["turn:34-220-69-168.t-67e8ec01.kinesisvideo.us-west-2.amazonaws.com:443?transport=udp","turns:34-220-69-168.t-67e8ec01.kinesisvideo.us-west-2.amazonaws.com:443?transport=udp","turns:34-220-69-168.t-67e8ec01.kinesisvideo.us-west-2.amazonaws.com:443?transport=tcp"],"Username":"1610088702:djE6YXJuOmF3czpraW5lc2lzdmlkZW86dXMtd2VzdC0yOjg0ODE2MTE2OTM1NDpjaGFubmVsL1NjYXJ5VGVzdENoYW5uZWwvMTYwNjkwMTI5ODQ4NA=="}]}
			* */
			MS_lOGD("/v1/get-ice-server-config recv data=",body.dump().c_str());
			json IceServerList = {
                "IceServerList",{
                        {
                            {"Password" , "H8TZ5pAjOkalNVQJ5UgQt3Godj3URm/3lsfcJYLMwOs="},
                            {"Ttl" , 300},
                            {"Uris" , {
                                "turn:192.168.1.192?transport=udp",
                                "turns:192.168.1.192?transport=udp"
                            },
                            {"Username" , "aa"}
                        }
                    }
                }
			};
			room = m_rooms["1234569"];
			//room->websocket = webSocketClient;
			//res.status(200).json(IceServerList);
		}

}

/**
 * Create a protoo WebSocketServer to allow WebSocket connections from browsers.
 */
void SfuServer::runProtooWebSocketServer()
{
//	MS_lOGD("running protoo WebSocketServer...");
//	// Handle connections from clients.
//
//    protooWebsockServer->on("connectionrequest", [&](std::string&& roomid, std::string&& peerid, WebSocketTransport* transport)
//	{
//        //MS_lOGD("[server] on connectionrequest info=${JSON.stringify(info)}")
//		// The client indicates the roomId and peerId in the URL query.
//		//auto u = url.parse(info.request.url, true);
//        auto roomId = roomid;//u.query["roomId"];
//        auto peerId = peerid;//u.query["peerId"];
//
//    	auto dvr = true;//u.query["dvr"];
//
//		if (roomId.empty() || peerId.empty())
//		{
//			//reject(400, "Connection request without roomId and/or peerId");
//
//			return;
//		}
//
//		MS_lOGD(
//			"protoo connection request [roomId:%s, peerId:%s]",
//			roomId.c_str(), peerId.c_str());
//
//		// Serialize this code into the queue to avoid that two peers connecting at
//		// the same time with the same roomId create two separate rooms with same
//		// roomId.
//        auto room = getOrCreateRoom(roomId );
//          if(dvr)  {
//            room->dvr = true;
//          }
//
//
//        // Accept the protoo WebSocket connection.
//       // auto protooWebSocketTransport = accept(json({}));
//
//       room->handleProtooConnection(peerId,nullptr, transport);
//
////		queue.push(async () =>
////		{
////			auto room = getOrCreateRoom(roomId );
////              if(dvr)  {
////                room->dvr = true;
////              }
////
////
////			// Accept the protoo WebSocket connection.
////			auto protooWebSocketTransport = accept();
////
////			room->handleProtooConnection(peerId, protooWebSocketTransport);
////		})
////			.catch((error) =>
////			{
////				logger.error("room creation or room joining failed:%o", error);
////
////				reject(error);
////			});
//	});
//    protooWebsockServer->runWebSocketServer();
}
/**
 * Get a Room instance (or create one if it does not exist).
 */
std::shared_ptr<Room> SfuServer::getOrCreateRoom(const oatpp::String& roomId)
{
	std::lock_guard<std::mutex> lock(m_roomsMutex);
	std::shared_ptr<Room> &room = m_rooms[roomId];

	// If the Room does not exist create a new one.
	if (!room)
	{
		auto mediasoupWorker = getMediasoupWorker();
        MS_lOGI("create() config.mediasoup.routerOptions=%s", this->m_pConfig->mediasoup.routerOptions.dump().c_str());
        json mediaCodecs = this->m_pConfig->mediasoup.routerOptions["mediaCodecs"];
        // Create a mediasoup Router.   
        RouterOptions routerOptions;
        routerOptions.mediaCodecs = mediaCodecs.get<std::vector<RtpCodecCapability>>();       
        auto mediasoupRouter =  mediasoupWorker->createRouter(routerOptions);
        // Create room instance
		room = std::make_shared<Room>(roomId, mediasoupRouter);
        room->setConfig(this->m_pConfig);
		//add to room map
		m_rooms[roomId] = room;		
	}
	return room;

	// if (!room)
	// {
	// 	MS_lOGD("creating a new Room [roomId:%s]", roomId.c_str());

	// 	auto mediasoupWorker = getMediasoupWorker();
    // //room->setConfig(config);
	// 	room = Room::create( mediasoupWorker, roomId );
   
	// 	rooms[roomId] = room;
	// 	room->on("close",[&]()
    //     {
	// 		MS_lOGD("getOrCreateRoom rooms delete [roomId:%s]", roomId.c_str());
	// 		rooms.erase(roomId);
	// 	});
	// }

	// return room;
}
void SfuServer::initMediasoup()
{
    mediasoup = mediasoup::CreateMediasoup();
    mediasoup->Init();
	mediasoup::RtpCapabilities rtpCapabilities = mediasoup->GetSupportedRtpCapabilities();
	for (auto & item : rtpCapabilities.headerExtensions) {
		//std::cout << "headerExtensions.uri:" << item.uri << std::endl;
	}

}
void SfuServer::initWorker(int consumerFd,int producerFd,int payloadConsumerFd,int payloadProducerFd)
{
    workerSettings.consumerFd=consumerFd;
    workerSettings.producerFd=producerFd;
    workerSettings.payloadConsumerFd=payloadConsumerFd;
    workerSettings.payloadProducerFd=payloadProducerFd;
  	auto worker = mediasoup->CreateWorker(&myWorkerObserver, workerSettings);
    workers.push_back(worker);
}
json SfuServer::getStringFromBase64(std::string payload)
{
    // Base64 base;
    // auto dec = base.Decode(payload.c_str(),(int)payload.length());
    // return json::parse(dec);
    return json();
}

void SfuServer::runPingLoop(const std::chrono::duration<v_int64, std::micro>& interval) {

  while(true) {

    std::chrono::duration<v_int64, std::micro> elapsed = std::chrono::microseconds(0);
    auto startTime = std::chrono::system_clock::now();

    do {
      std::this_thread::sleep_for(interval - elapsed);
      elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - startTime);
    } while (elapsed < interval);

    std::lock_guard<std::mutex> lock(m_roomsMutex);
    for (const auto &room : m_rooms) {
      room.second->pingAllPeers();
    }

  }

}

void SfuServer::onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket, const std::shared_ptr<const ParameterMap>& params) {

  //++ m_statistics->EVENT_PEER_CONNECTED;

  auto roomId = params->find("roomId")->second;
  auto peerId = params->find("peerId")->second;
  auto forceH264 = params->find("forceH264")->second;
  auto forceVP9 = params->find("forceVP9")->second;

  auto room = getOrCreateRoom(roomId);

  //check whether peerId existed
   auto existingPeer = room->getPeerById(peerId->std_str());
  if(existingPeer){
	  MS_lOGW("[Room] handleProtooConnection() | there is already a protoo Peer with same peerid(), closing it [peerId:%s]",
              peerId->std_str().c_str());
            // TODO:should close
			//existingPeer->close();
	  return;
  }
 //TODO: uncomment below 3 lines
  auto peer = std::make_shared<Peer>(socket, room, peerId->std_str(), peerId->std_str());
  socket->setListener(peer);
  room->addPeer(peer);

//   protooPeer->on("request", [&, peerId](json request, std::function<void(json data)> const &accept, std::function<void(int errorCode, std::string errorReason)> const &reject)
// 				 {
// 					 std::shared_ptr<Peer> peer1 = this->_peers[peerId];
// 					 MS_lOGD(
// 						 "protoo Peer  event [method:%s, peerId:%s peer=%x]",
// 						 request["method"].dump().c_str(), peer1->id.c_str(), peer1.get());

// 					 this->_handleProtooRequest(peer1, request, accept, reject);
// 				 });

//   protooPeer->on("close", [&]()
// 				 {
// 					 if (this->_closed)
// 						 return;

// 					 MS_lOGD("protoo Peer event [peerId:%s]", peer->id.c_str());

// 					 // If the Peer was joined, notify all Peers.
// 					 if (peer->data.joined)
// 					 {
// 						 for (auto otherPeer : this->_getJoinedPeers(peer))
// 						 {
// 							 json data = {{"peerId", peer->id}};
// 							 otherPeer->notify("peerClosed", data);
// 						 }
// 					 }

// 					 // Iterate and close all mediasoup Transport associated to this Peer, so all
// 					 // its Producers and Consumers will also be closed.
// 					 for (auto kv : peer->data.transports)
// 					 {
// 						 //auto transportId        = kv.first;
// 						 auto transport = kv.second;
// 						 transport->close();
// 					 }

// 					 // If this is the latest Peer in the room, close the room.
// 					 if (this->_peers.size() == 0)
// 					 {
// 						 MS_lOGI(
// 							 "last Peer in the room left, closing the room [roomId:%s]",
// 							 this->_roomId.c_str());

// 						 this->close();
// 					 }
// 				 });
}

void SfuServer::onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket>& socket) {

  //++ m_statistics->EVENT_PEER_DISCONNECTED;

  auto peer = std::static_pointer_cast<Peer>(socket->getListener());
    if(!peer){
        return;
    }
  auto room = peer->getRoom();

  room->removePeerById(peer->getPeerId());
  //room->goodbyePeer(peer);
  peer->invalidateSocket();

  if(room->isEmpty()) {
    deleteRoom(room->getId());
  }

}

void SfuServer::deleteRoom(const oatpp::String& roomId){
    std::lock_guard<std::mutex> lock(m_roomsMutex);
      m_rooms.erase(roomId);
}

std::string Base64::Encode(const unsigned char * str,int bytes) {
    int num = 0,bin = 0,i;
    std::string _encode_result;
    const unsigned char * current;
    current = str;
    while(bytes > 2) {
        _encode_result += _base64_table[current[0] >> 2];
        _encode_result += _base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
        _encode_result += _base64_table[((current[1] & 0x0f) << 2) + (current[2] >> 6)];
        _encode_result += _base64_table[current[2] & 0x3f];

        current += 3;
        bytes -= 3;
    }
    if(bytes > 0)
    {
        _encode_result += _base64_table[current[0] >> 2];
        if(bytes%3 == 1) {
            _encode_result += _base64_table[(current[0] & 0x03) << 4];
            _encode_result += "==";
        } else if(bytes%3 == 2) {
            _encode_result += _base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
            _encode_result += _base64_table[(current[1] & 0x0f) << 2];
            _encode_result += "=";
        }
    }
    return _encode_result;
}
std::string Base64::Decode(const char *str,int length) {
       //解码表
    const int DecodeTable[] =
    {
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -2, -2, -1, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 62, -2, -2, -2, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -2, -2, -2, -2, -2, -2,
        -2,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -2, -2, -2, -2, -2,
        -2, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2
    };
    int bin = 0,i=0,pos=0;
    std::string _decode_result;
    const char *current = str;
    char ch;
    while( (ch = *current++) != '\0' && length-- > 0 )
    {
        if (ch == base64_pad) { // 当前一个字符是“=”号
            /*
            先说明一个概念：在解码时，4个字符为一组进行一轮字符匹配。
            两个条件：
                1、如果某一轮匹配的第二个是“=”且第三个字符不是“=”，说明这个带解析字符串不合法，直接返回空
                2、如果当前“=”不是第二个字符，且后面的字符只包含空白符，则说明这个这个条件合法，可以继续。
            */
            if (*current != '=' && (i % 4) == 1) {
                return NULL;
            }
            continue;
        }
        ch = (char)DecodeTable[ch];
        //这个很重要，用来过滤所有不合法的字符
        if (ch < 0 ) { /* a space or some other separator character, we simply skip over */
            continue;
        }
        switch(i % 4)
        {
            case 0:
                bin = ch << 2;
                break;
            case 1:
                bin |= ch >> 4;
                _decode_result += bin;
                bin = ( ch & 0x0f ) << 4;
                break;
            case 2:
                bin |= ch >> 2;
                _decode_result += bin;
                bin = ( ch & 0x03 ) << 6;
                break;
            case 3:
                bin |= ch;
                _decode_result += bin;
                break;
        }
        i++;
    }
    return _decode_result;
}
                                   
