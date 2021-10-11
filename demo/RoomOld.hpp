#pragma once
#include "IMediasoup.hpp"
#include "IWorker.hpp"
#include "Log.hpp"
#include "Consumer.hpp"
#include "Router.hpp"
#include "Log.hpp"
#include "Config.hpp"
#include "AudioLevelObserver.hpp"
#include "EnhancedEventEmitter.hpp"
#include "json.hpp"
#include "Room.h"   //protoo  
#include "Peer.hpp"
#include "Transfmt.hpp"
#include "sdp/SdpOffer.hpp"
using json = nlohmann::json;
class Base64{
private:
    std::string _base64_table;
    static const char base64_pad = '=';public:
    Base64()
    {
        _base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; /*这是Base64编码使用的标准字典*/
    }
    /**
     * 这里必须是unsigned类型，否则编码中文的时候出错
     */
    std::string Encode(const unsigned char * str,int bytes);
    std::string Decode(const char *str,int bytes);
    void Debug(bool open = true);
};
/**
 * Room class.
 *
 * This is not a "mediasoup Room" by itself, by a custom class that holds
 * a protoo Room (for signaling with WebSocket clients) and a mediasoup Router
 * (for sending and receiving media to/from those WebSocket peers).
 */

void to_json(json& j, const _device& st);
void from_json(const json& j, _device& st);
struct pairInfo
{
    std::string id ;//
    std::string kind ;//
};
void to_json(json& j, const pairInfo& st);
void from_json(const json& j, pairInfo& st);
 
void to_json(json& j, const PeerInfo& st);
void from_json(const json& j, PeerInfo& st);
class Room : public  EnhancedEventEmitter
{
public:
    //Config
    Config config;
    
    //record
    Transfmt transfmt;
    
	// Room id.
		// @type {String}
		std::string _roomId;
    bool dvr = false;

		// Closed flag.
		// @type {Boolean}
		bool _closed = false;

		// protoo Room instance.
		// @type {protoo.Room}
    std::shared_ptr<protoo::Room> _protooRoom;
    std::unordered_map<std::string,shared_ptr<Peer> > _peers;
		// Map of broadcasters indexed by id. Each Object has:
		// - {String} id
		// - {Object} data
		//   - {String} displayName
		//   - {Object} device
		//   - {RTCRtpCapabilities} rtpCapabilities
		//   - {Map<String, mediasoup.Transport>} transports
		//   - {Map<String, mediasoup.Producer>} producers
		//   - {Map<String, mediasoup.Consumers>} consumers
		//   - {Map<String, mediasoup.DataProducer>} dataProducers
		//   - {Map<String, mediasoup.DataConsumers>} dataConsumers
		// @type {Map<String, Object>}
		//this->_broadcasters = new Map();
    std::unordered_map<std::string,shared_ptr<Peer> > _broadcasters;
        // @type {Map<String, Object>}
        //this->_bridges = new Map();
    std::unordered_map<std::string,shared_ptr<Peer> > _bridges;
		// mediasoup Router instance.
		// @type {mediasoup.Router}
    std::shared_ptr<Router> _mediasoupRouter ;//= mediasoupRouter;

		// mediasoup AudioLevelObserver.
		// @type {mediasoup.AudioLevelObserver}
    std::shared_ptr<AudioLevelObserver>  _audioLevelObserver;

		// DataChannel bot.
		// @type {Bot}
    void *_bot;

		// Network throttled.
		// @type {Boolean}
		bool _networkThrottled = false;

		// Handle audioLevelObserver.
		//this->_handleAudioLevelObserver();

		// For debugging.
		//global.audioLevelObserver = this->_audioLevelObserver;
		//global.bot = this->_bot;
 
	void setConfig(Config &config1) {
     
        this->config.initConfig();
  }
	Room(std::string &roomId, std::shared_ptr<protoo::Room> protooRoom, std::shared_ptr<Router> mediasoupRouter, std::shared_ptr<AudioLevelObserver> audioLevelObserver)
	{
		//super();
		//this->setMaxListeners(1000);

		// Room id.
		// @type {String}
		this->_roomId = roomId;

		// Closed flag.
		// @type {Boolean}
		this->_closed = false;

		// protoo Room instance.
		// @type {protoo.Room}
		this->_protooRoom = protooRoom;

		// Map of broadcasters indexed by id. Each Object has:
		// - {String} id
		// - {Object} data
		//   - {String} displayName
		//   - {Object} device
		//   - {RTCRtpCapabilities} rtpCapabilities
		//   - {Map<String, mediasoup.Transport>} transports
		//   - {Map<String, mediasoup.Producer>} producers
		//   - {Map<String, mediasoup.Consumers>} consumers
		//   - {Map<String, mediasoup.DataProducer>} dataProducers
		//   - {Map<String, mediasoup.DataConsumers>} dataConsumers
		// @type {Map<String, Object>}
		//this->_broadcasters = new Map();

    // @type {Map<String, Object>}
   // this->_bridges = new Map();
    
		// mediasoup Router instance.
		// @type {mediasoup.Router}
		this->_mediasoupRouter = mediasoupRouter;

		// mediasoup AudioLevelObserver.
		// @type {mediasoup.AudioLevelObserver}
		this->_audioLevelObserver = audioLevelObserver;

		// DataChannel bot.
		// @type {Bot}
		//this->_bot = bot;

		// Network throttled.
		// @type {Boolean}
		this->_networkThrottled = false;

		// Handle audioLevelObserver.
		this->_handleAudioLevelObserver();

    config.initConfig();

		// For debugging.
		//global.audioLevelObserver = this->_audioLevelObserver;
		//global.bot = this->_bot;
	}
    /**
     * Factory function that creates and returns Room instance.
     *
     * @async
     *
     * @param {mediasoup.Worker} mediasoupWorker - The mediasoup Worker in which a new
     *   mediasoup Router must be created.
     * @param {String} roomId - Id of the Room instance.
     */
    static std::shared_ptr<Room>  create(std::shared_ptr<mediasoup::IWorker> mediasoupWorker, std::string &roomId )
    {
        MS_lOGI("create() [roomId:%s]", roomId.c_str());

        // Create a protoo Room instance.
        std::shared_ptr<protoo::Room> protooRoom = std::make_shared<protoo::Room>();

        // Router media codecs.
        //const { mediaCodecs } = config.mediasoup.routerOptions;
        Config config;
        config.initConfig();
        MS_lOGI("create() config.mediasoup.routerOptions=%s", config.mediasoup.routerOptions.dump().c_str());
        json mediaCodecs = config.mediasoup.routerOptions["mediaCodecs"];
        // Create a mediasoup Router.
    
        RouterOptions routerOptions;
        routerOptions.mediaCodecs = mediaCodecs.get<std::vector<RtpCodecCapability>>();
        
        auto mediasoupRouter =  mediasoupWorker->createRouter(routerOptions);

        // Create a mediasoup AudioLevelObserver.
        AudioLevelObserverOptions options;
        options.maxEntries = 1;
        options.threshold  = -80;
        options.interval   = 800;
        auto audioLevelObserver =  mediasoupRouter->createAudioLevelObserver( options);

        //const bot =  Bot.create({ mediasoupRouter });

        return std::make_shared<Room>(
                roomId,
                protooRoom,
                mediasoupRouter,
                audioLevelObserver
         );
    }

	/**
	 * Closes the Room instance by closing the protoo Room and the mediasoup Router.
	 */
	void close()
	{
		MS_lOGD("close()");

		this->_closed = true;

		// Close the protoo Room.
		//this->_protooRoom->close();

		// Close the mediasoup Router.
		this->_mediasoupRouter->close();

		// Close the Bot.
		//this->_bot.close();

		// Emit "close" event.
		this->emit("close");

		// Stop network throttling.
		if (this->_networkThrottled)
		{
			//throttle.stop({})
			//	
		}
	}

	void logStatus()
	{
/*
		MS_lOGI(
			"logStatus() [roomId:%s, protoo Peers:%s, mediasoup Transports:%s]",
			this->_roomId,
			this->_protooRoom->peers.length,
			this->_mediasoupRouter->_transports.size); // NOTE: Private API.
*/
	}

	/**
	 * Called from server.js upon a protoo WebSocket connection request from a
	 * browser.
	 *
	 * @param {String} peerId - The id of the protoo peer to be created.
	 * @param {Boolean} consume - Whether this peer wants to consume from others.
	 * @param {protoo.WebSocketTransport} protooWebSocketTransport - The associated
	 *   protoo WebSocket transport->
	 */
	void handleProtooConnection(std::string &peerId, std::shared_ptr<Consumer> consume, WebSocketTransport* protooWebSocketTransport )
	{

		auto  existingPeer = this->_protooRoom->getPeer(peerId);

		if (existingPeer)
		{
			MS_lOGW(
				"handleProtooConnection() | there is already a protoo Peer with same peerid(), closing it [peerId:%s]",
                    peerId.c_str());

			existingPeer->close();
		}

		std::shared_ptr<protoo::Peer> protooPeer;
        std::shared_ptr<Peer> peer;
        if(this->_peers.find(peerId) != this->_peers.end())
        {
            MS_lOGW(
                "handleProtooConnection() | there is already a Peer with same peerid() [peerId:%s]",
                    peerId.c_str());
            peer = this->_peers[peerId];
        } else
        {
            MS_lOGW(
                "handleProtooConnection() | new a Peer [peerId:%s]",
                    peerId.c_str());
            peer = std::make_shared<Peer>();
        }
      

		// Create a new protoo Peer with the given peerId.
		try
		{
            protooPeer = this->_protooRoom->createPeer(peerId, protooWebSocketTransport);
            peer->protooPeer = protooPeer;
            this->_peers[peerId] = peer;
		}
		catch (const char *error)
		{
			MS_lOGE("protooRoom.createPeer() failed:%s", error);
            return;
		}

		// Use the peer->data object to store mediasoup related objects.

        // Not joined after a custom protoo "join" request is later received.
        peer->id = peerId;
        peer->data.consume = consume;
       // peer->data.joined = false;

        if(!protooPeer)
        {
            MS_lOGE("protooRoom.createPeer() failed: protooPeer == null");
            return;
        }
        protooPeer->on("request",[&,peerId](json request, std::function<void(json data)> const& accept, std::function<void(int errorCode, std::string errorReason)> const& reject)
		{
            std::shared_ptr<Peer> peer1 = this->_peers[peerId];
			MS_lOGD(
				"protoo Peer  event [method:%s, peerId:%s peer=%x]",
				request["method"].dump().c_str(), peer1->id.c_str(),peer1.get());

            this->_handleProtooRequest(peer1, request, accept, reject);
//				.catch((error) =>
//				{
//					MS_lOGE("request failed:%o", error);
//
//					reject(error);
//				});
		});

        protooPeer->on("close",[&]()
		{
			if (this->_closed)
				return;

			MS_lOGD("protoo Peer event [peerId:%s]", peer->id.c_str());

			// If the Peer was joined, notify all Peers.
			if (peer->data.joined)
			{
				for (auto otherPeer : this->_getJoinedPeers( peer ))
				{
                    json data = { {"peerId", peer->id} };
                    otherPeer->notify("peerClosed",data );
						
				}
			}

			// Iterate and close all mediasoup Transport associated to this Peer, so all
			// its Producers and Consumers will also be closed.
			for (auto kv : peer->data.transports)
			{
                //auto transportId        = kv.first;
                auto transport   = kv.second;
				transport->close();
			}

			// If this is the latest Peer in the room, close the room.
			if (this->_peers.size() == 0)
			{
				MS_lOGI(
					"last Peer in the room left, closing the room [roomId:%s]",
					this->_roomId.c_str());

				this->close();
			}
		});

	}

	RtpCapabilities getRouterRtpCapabilities()
	{
		return this->_mediasoupRouter->rtpCapabilities();
	}
  std::shared_ptr<Peer>  getPeerInfo(std::string flag,std::string &id, std::string &displayName, json  &device, RtpCapabilities &rtpCapabilities)
    {
        std::shared_ptr<Peer>  info = std::make_shared<Peer>();
        info->id = id;
        info->data.displayName = displayName;
        info->data.device.flag = flag;
        info->data.device.name = device["name"];
        info->data.device.version = device["version"];
        info->data.rtpCapabilities = rtpCapabilities;
        return info;
    }
    
    json getDevice(Peer & info) {
        json device = json::object();
        device["flag"] = info.data.device.flag;
        device["name"] = info.data.device.name;
        device["version"] = info.data.device.version;
        return device;
    }
	/**
	 * Create a Broadcaster. This is for HTTP API requests (see server.js).
	 *
	 * @async
	 *
	 * @type {String} id - Broadcaster id.
	 * @type {String} displayName - Descriptive name.
	 * @type {Object} [device] - Additional info with name, version and flags fields.
	 * @type {RTCRtpCapabilities} [rtpCapabilities] - Device RTP capabilities.
	 */
    std::vector<PeerInfo > createBroadcaster(std::string id, std::string displayName, json  device, RtpCapabilities &rtpCapabilities )
	{
//		if (typeof id != "string" || !id)
//			MS_THROW_lOG("missing body.id");
//		else if (typeof displayName != "string" || !displayName)
//			MS_THROW_lOG("missing body.displayName");
//		else if (typeof device.name != "string" || !device.name)
//			MS_THROW_lOG("missing body.device.name");
//		else if (rtpCapabilities && typeof rtpCapabilities != "object")
//			MS_THROW_lOG("wrong body.rtpCapabilities");
//
//		if (this->_broadcasters.has(id))
//			MS_THROW_lOG("broadcaster with id id=%s already exists");

//		auto  broadcaster =
//		{
//			id(),
//			data :
//			{
//				displayName,
//				device :
//				{
//					flag    : "broadcaster",
//					name    : device.name || "Unknown device",
//					version : device.version
//				},
//				rtpCapabilities,
//				transports    : new Map(),
//				producers     : new Map(),
//				consumers     : new Map(),
//				dataProducers : new Map(),
//				dataConsumers : new Map()
//			}
//		};
    auto broadcaster = getPeerInfo("broadcaster",id, displayName, device, rtpCapabilities);
		// Store the Broadcaster into the map.
		this->_broadcasters[broadcaster->id] = broadcaster;

		// Notify the new Broadcaster to all Peers.
		for (auto  otherPeer : this->_getJoinedPeers())
		{
			otherPeer->notify(
				"newPeer",
				{
					{"id"          , broadcaster->id},
					{"displayName" , broadcaster->data.displayName},
					{"device"      , broadcaster->data.device}
            });
				//
		}

		// Reply with the list of Peers and their Producers.
		std::vector<PeerInfo> peerInfos;
		auto  joinedPeers = this->_getJoinedPeers();

		// Just fill the list of Peers if the Broadcaster provided its rtpCapabilities.
		if (true)//rtpCapabilities)
		{
			for (auto  joinedPeer : joinedPeers)
			{
//				const peerInfo =
//				{
//					id          : joinedPeer->id(),
//					displayName : joinedPeer->data.displayName,
//					device      : joinedPeer->data.device,
//					producers   : []
//				};
                PeerInfo peerInfo;
                peerInfo.id          = joinedPeer->id;
                peerInfo.displayName = joinedPeer->data.displayName;
                peerInfo.device      = joinedPeer->data.device;
                //info.producers   = []
                //for (auto producer : joinedPeer->data.producers)
                //{
                for (const auto& kv : joinedPeer->data.producers)
                {
                    auto producerId        = kv.first;
                    auto &producer = kv.second;
				
					// Ignore Producers that the Broadcaster cannot consume.
					if (
						!this->_mediasoupRouter->canConsume(
								producer->id(),
								rtpCapabilities
							)
					)
					{
						continue;
					}
             
						peerInfo.producers.push_back({
							{"id"   , producer->id()},
							{"kind" , producer->kind()}
						});
				}

				peerInfos.push_back(peerInfo);
			}
		}

		return peerInfos;
	}

	/**
	 * Delete a Broadcaster.
	 *
	 * @type {String} broadcasterId
	 */
	void deleteBroadcaster( std::string broadcasterId )
	{
		auto  broadcaster = this->_broadcasters[broadcasterId];

		if (!broadcaster)
			MS_THROW_lOG("broadcaster with id broadcasterId=%s does not exist",broadcasterId.c_str());

		for (auto kv : broadcaster->data.transports)
		{
            //auto transportId        = kv.first;
            auto transport   = kv.second;
			transport->close();
		}

		this->_broadcasters.erase(broadcasterId);

		for (auto  peer :this->_getJoinedPeers())
		{
            peer->notify("peerClosed", { {"peerId", broadcasterId }});
		}
  }
    std::shared_ptr<Peer>  getPeerFrom(std::string &bridgeId) {
		  // Notify the new Broadcaster to all Peers.
		  auto  bridge = this->_bridges[bridgeId];
		  return bridge;
    }
//    RtpCapabilities  getLocalSdp()
//	  {
//		return this->_mediasoupRouter->rtpCapabilities();
//	  }
    json  getLocalSdp()
	  {
        json jrtp = this->_mediasoupRouter->rtpCapabilities();
		    return jrtp;
	  }
	/**
	 * Create a Bridge. This is for HTTP API requests (see server.js).
	 *
	 * @async
	 *
	 * @type {String} id - Broadcaster id.
	 * @type {String} displayName - Descriptive name.
	 * @type {Object} [device] - Additional info with name, version and flags fields.
	 * @type {RTCRtpCapabilities} [rtpCapabilities] - Device RTP capabilities.
	 */
	json createBridge(std::string id, std::string displayName, json & device, RtpCapabilities &rtpCapabilities)
	{
//		if (typeof id != "string" || !id)
//			MS_THROW_lOG("missing body.id");
//		else if (typeof displayName != "string" || !displayName)
//			MS_THROW_lOG("missing body.displayName");
//		else if (typeof device.name != "string" || !device.name)
//			MS_THROW_lOG("missing body.device.name");
//		else if (rtpCapabilities && typeof rtpCapabilities != "object")
//			MS_THROW_lOG("wrong body.rtpCapabilities");
//
//		if (this->_bridges.has(id))
//			MS_THROW_lOG("bridge with id id=%s already exists");

		// auto  bridge =
		// 	{
		// 		id(),
		// 		data :
		// 			{
		// 				displayName,
		// 				device :
		// 					{
		// 						flag    : "bridge",
		// 						name    : device.name || "Unknown device",
		// 						version : device.version
		// 					},
		// 				rtpCapabilities,
		// 				transports    : new Map(),
		// 				producers     : new Map(),
		// 				consumers     : new Map(),
		// 				dataProducers : new Map(),
		// 				dataConsumers : new Map()
		// 			}
		// 	};
    auto bridge = getPeerInfo("bridge",id, displayName, device, rtpCapabilities);
		// Store the bridge into the map.
		this->_bridges[bridge->id]= bridge;

        // Notify the new Broadcaster to all Peers.
        for (auto  otherPeer : this->_getJoinedPeers())
        {
            otherPeer->notify(
                "newPeer",
                {
                    {"id"          , bridge->id},
                    {"displayName" , bridge->data.displayName},
                    {"device"      , bridge->data.device}
            });
                //
        }

        // Reply with the list of Peers and their Producers.
        std::vector<PeerInfo> peerInfos;
        auto  joinedPeers = this->_getJoinedPeers();

        // Just fill the list of Peers if the Broadcaster provided its rtpCapabilities.
        if (true)//rtpCapabilities)
        {
            for (auto  joinedPeer : joinedPeers)
            {
//                const peerInfo =
//                {
//                    id          : joinedPeer->id(),
//                    displayName : joinedPeer->data.displayName,
//                    device      : joinedPeer->data.device,
//                    producers   : []
//                };
                PeerInfo peerInfo;
                peerInfo.id          = joinedPeer->id;
                peerInfo.displayName = joinedPeer->data.displayName;
                peerInfo.device      = joinedPeer->data.device;
                //info.producers   = []
                //for (auto producer : joinedPeer->data.producers)
                //{
                for (const auto& kv : joinedPeer->data.producers)
                {
                    auto producerId        = kv.first;
                    auto &producer = kv.second;
                
                    // Ignore Producers that the Broadcaster cannot consume.
                    if (
                        !this->_mediasoupRouter->canConsume(
                                producer->id(),
                                rtpCapabilities
                            )
                    )
                    {
                        continue;
                    }
             
                        peerInfo.producers.push_back({
                            {"id"   , producer->id()},
                            {"kind" , producer->kind()}
                        });
                }

                peerInfos.push_back(peerInfo);
            }
        }

        return peerInfos;
	}

	
	/**
	 * Create a mediasoup Transport associated to a Broadcaster. It can be a
	 * PlainTransport or a WebRtctransport->
	 *
	 * @async
	 *
	 * @type {String} broadcasterId
	 * @type {String} type - Can be "plain" (PlainTransport) or "webrtc"
	 *   (WebRtcTransport).
	 * @type {Boolean} [rtcpMux=false] - Just for PlainTransport, use RTCP mux.
	 * @type {Boolean} [comedia=true] - Just for PlainTransport, enable remote IP:port
	 *   autodetection.
	 * @type {Object} [sctpCapabilities] - SCTP capabilities
	 */
	json createBroadcasterTransport(
		//{
			std::string broadcasterId,
            std::string type,
			bool rtcpMux,// = false,
			bool comedia,// = true,
            SctpCapabilities &sctpCapabilities
		//}
  )
	{
		auto  broadcaster = this->_broadcasters[broadcasterId];

		if (!broadcaster)
			MS_THROW_lOG("broadcaster with id broadcasterId=%s does not exist",broadcasterId.c_str());

		if (true)
		{
			if(type == "webrtc")
			{
                json webRtcTransportOptions =
				{
					config.mediasoup.webRtcTransportOptions,
                    {"enableSctp"     , (sctpCapabilities.numStreams.OS != 0)},
                        {"numSctpStreams" , sctpCapabilities.numStreams}
				};
                WebRtcTransportOptions options = webRtcTransportOptions;
				auto transport =  this->_mediasoupRouter->createWebRtcTransport(options);

				// Store it.
				broadcaster->data.transports[transport->id()]= transport;

				return {
					{"id"             , transport->id()},
					{"iceParameters"  , transport->iceParameters()},
					{"iceCandidates"  , transport->iceCandidates()},
					{"dtlsParameters" , transport->dtlsParameters()},
					{"sctpParameters" , transport->sctpParameters()}
				};
			}else
			if(type == "plain")
			{
				json plainTransportOptions =
				{
					config.mediasoup.plainTransportOptions,
                    {"rtcpMux" , rtcpMux},
                    {"comedia" , comedia}
				};
                PlainTransportOptions options = plainTransportOptions;
				auto transport =  this->_mediasoupRouter->createPlainTransport(options);

				// Store it.
				broadcaster->data.transports[transport->id()]= transport;

				return {
					{"id"       , transport->id()},
					{"ip"       , transport->tuple().localIp},
					{"port"     , transport->tuple().localPort},
					{"rtcpPort" , transport->rtcpTuple().localPort}
				};
			} else 
			{
				MS_THROW_lOG("invalid type");
			}
		}
	}

	/**
	 * Connect a Broadcaster mediasoup WebRtctransport->
	 *
	 * @async
	 *
	 * @type {String} broadcasterId
	 * @type {String} transportId
	 * @type {RTCDtlsParameters} dtlsParameters - Remote DTLS parameters.
	 */
	void connectBroadcasterTransport(
		//{
			std::string broadcasterId,
			std::string transportId,
            DtlsParameters &dtlsParameters
	//	}
	)
	{
		auto  broadcaster = this->_broadcasters[broadcasterId];

		if (!broadcaster)
			MS_THROW_lOG("broadcaster with id broadcasterId=%s does not exist",broadcasterId.c_str());

		auto transport = broadcaster->data.transports[transportId];

		if (!transport)
			MS_THROW_lOG("transport with id transportId=%s does not exist",transportId.c_str());

        /*
		if (transport->constructor.name != "WebRtcTransport")
		{
			MS_THROW_lOG(
				"transport with id transportId=%s is not a WebRtcTransport",transportId.c_str());
		}*/
        json options = { {"dtlsParameters",dtlsParameters} };
        transport->connect(options);
	}

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
	)
	{
		auto  broadcaster = this->_broadcasters[broadcasterId];

		if (!broadcaster)
			MS_THROW_lOG("broadcaster with id broadcasterId=%s does not exist",broadcasterId.c_str());

		auto transport = broadcaster->data.transports[transportId];

		if (!transport)
			MS_THROW_lOG("transport with id transportId=%s does not exist",transportId.c_str());

        ProducerOptions options;// = {
            options.kind = kind;
            options.rtpParameters = rtpParameters;
        //};
        auto producer =
			 transport->produce(options);

		// Store it.
		broadcaster->data.producers[producer->id()]= producer;

		// Set Producer events.
		// producer->on("score",[&]() //, (score) =>
		// {
		// 	MS_lOGD(
		// 		"broadcaster producer "score" event [producerId:%s, score:%o]",
		// 		producer->id(), score);
		// });

		producer->on("videoorientationchange",[&]( int videoOrientation ) //, (videoOrientation) =>
		{
            //auto videoOrientation = e.value().get<int>();
			MS_lOGD(
				"broadcaster producer videoorientationchange event [producerId:%s, videoOrientation:%o]",
				producer->id().c_str(), videoOrientation);
		});

		// Optimization: Create a server-side Consumer for each Peer.
		for (auto  peer :this->_getJoinedPeers())
		{
			this->_createConsumer(
				
					/*consumerPeer :*/ peer,
					/*producerPeer :*/ broadcaster,
					producer
				);
            
            if(this->dvr) {
                
                bool hasVideo = false;
                bool hasAudio = false;
                for (auto &kv : peer->data.producers) {
                    auto producerPeer = kv.second;
                    if(producerPeer->kind() == "video") {
                        hasVideo = true;
                    }
                    if(producerPeer->kind() == "audio") {
                        hasAudio = true;
                    }
                }
                if(hasVideo && hasAudio) {
                    //mcuPublish(this->_mediasoupRouter, peer);
                    transfmt.startRecord(this->_mediasoupRouter, peer, producer,broadcasterId);
                }
            }
		}

		// Add into the audioLevelObserver.
		if (producer->kind() == "audio")
		{
            json options = {
                {"producerId",producer->id()}
            };
			this->_audioLevelObserver->addProducer(options);
		}
        
		return { 
      {"id", producer->id()} 
    };
	}

	/**
	 * Create a mediasoup Consumer associated to a Broadcaster.
	 *
	 * @async
	 *
	 * @type {String} broadcasterId
	 * @type {String} transportId
	 * @type {String} producerId
	 */
	json createBroadcasterConsumer(
		//{
			std::string broadcasterId,
			std::string transportId,
			std::string producerId
		//}
	)
	{
		auto  broadcaster = this->_broadcasters[broadcasterId];

		if (!broadcaster)
			MS_THROW_lOG("broadcaster with id broadcasterId=%s does not exist",broadcasterId.c_str());

		if (!broadcaster->data.rtpCapabilities)
			MS_THROW_lOG("broadcaster does not have rtpCapabilities");

		auto transport = broadcaster->data.transports[transportId];

		if (!transport)
            MS_THROW_lOG("transport with id transportId=%s does not exist",transportId.c_str());
    ConsumerOptions options;
    options.producerId = producerId;
    options.rtpCapabilities = broadcaster->data.rtpCapabilities;
		auto  consumer =  transport->consume(options);

		// Store it.
		broadcaster->data.consumers[consumer->id()]=consumer;

		// Set Consumer events.
		consumer->on("transportclose",[&]()
		{
			// Remove from its map.
			broadcaster->data.consumers.erase(consumer->id());
		});

		consumer->on("producerclose",[&]()
		{
			// Remove from its map.
			broadcaster->data.consumers.erase(consumer->id());
		});
//json j = {{"foo", 1}, {"bar", 2}};
		return {
			{"id"         , consumer->id()},
			{"producerId" , producerId},
			{"kind"       , consumer->kind()},
			{"rtpParameters", consumer->rtpParameters()},
			{"type"          , consumer->type()}
		};
	}

	/**
	 * Create a mediasoup DataConsumer associated to a Broadcaster.
	 *
	 * @async
	 *
	 * @type {String} broadcasterId
	 * @type {String} transportId
	 * @type {String} dataProducerId
	 */
	json createBroadcasterDataConsumer(
		//{
            std::string broadcasterId,
			std::string transportId,
			std::string dataProducerId
		//}
	)
	{
		auto  broadcaster = this->_broadcasters[broadcasterId];

		if (!broadcaster)
			MS_THROW_lOG("broadcaster with id broadcasterId=%s does not exist",broadcasterId.c_str());

		if (!broadcaster->data.rtpCapabilities)
			MS_THROW_lOG("broadcaster does not have rtpCapabilities");

		auto transport = broadcaster->data.transports[transportId];

		if (!transport)
            MS_THROW_lOG("transport with id transportId=%s does not exist",transportId.c_str());

        DataConsumerOptions options;
        options.dataProducerId = dataProducerId;
		auto dataConsumer =  transport->consumeData(options);

		// Store it.
		broadcaster->data.dataConsumers[dataConsumer->id()] =  dataConsumer;

		// Set Consumer events.
        dataConsumer->on("transportclose",[&]()
		{
			// Remove from its map.
			broadcaster->data.dataConsumers.erase(dataConsumer->id());
		});

        dataConsumer->on("dataproducerclose",[&]()
		{
			// Remove from its map.
			broadcaster->data.dataConsumers.erase(dataConsumer->id());
		});

		return {{
            {"id",  dataConsumer->id()}
		}};
	}

	/**
	 * Create a mediasoup DataProducer associated to a Broadcaster.
	 *
	 * @async
	 *
	 * @type {String} broadcasterId
	 * @type {String} transportId
	 */
	json createBroadcasterDataProducer(
	//	{
			std::string broadcasterId,
			std::string transportId,
			std::string label,
			std::string protocol,
      SctpStreamParameters &sctpStreamParameters,
			json appData
	//	}
	)
	{
		auto  broadcaster = this->_broadcasters[broadcasterId];

		if (!broadcaster)
			MS_THROW_lOG("broadcaster with id broadcasterId=%s does not exist",broadcasterId.c_str());

		// if (!broadcaster->data.sctpCapabilities)
		// 	MS_THROW_lOG("broadcaster does not have sctpCapabilities");

		auto transport = broadcaster->data.transports[transportId];

		if (!transport)
            MS_THROW_lOG("transport with id transportId=%s does not exist",transportId.c_str());
        DataProducerOptions option;
        option.sctpStreamParameters = sctpStreamParameters,
        option.label = label;
        option.protocol = protocol;
        option._appData = appData;
		auto dataProducer =  transport->produceData(option);

		// Store it.
		broadcaster->data.dataProducers[dataProducer->id()] = dataProducer;

		// Set Consumer events.
		dataProducer->on("transportclose",[&]( const event::event_t<json>& e )
		{
			// Remove from its map.
			broadcaster->data.dataProducers.erase(dataProducer->id());
		});

		// // Optimization: Create a server-side Consumer for each Peer.
		// for (auto  peer :this->_getJoinedPeers())
		// {
		// 	this->_createDataConsumer(
		// 		{
		// 			dataConsumerPeer : peer,
		// 			dataProducerPeer : broadcaster,
		// 			dataProducer: dataProducer
		// 		});
		// }

		return {
			{"id" , dataProducer->id()}
		};
	}
    /**
     * Delete a Broadcaster.
     *
     * @type {String} broadcasterId
     */
    void deleteBridge( std::string &bridgeId)
    {
        auto  bridge = this->_bridges[bridgeId];

        if (!bridge)
            MS_THROW_lOG("broadcaster with id bridgeId=%s does not exist",bridgeId.c_str());

        for (auto kv : bridge->data.transports)
        {
            //auto transportId        = kv.first;
            auto transport   = kv.second;
            transport->close();
        }

        this->_bridges.erase(bridgeId);

        for (auto  peer :this->_getJoinedPeers())
        {
            peer->notify("peerClosed", { {"peerId", bridgeId }});
        }
    }
    std::string getBridgeRoomId(std::string &bridgeId) {
        auto  bridge = this->_bridges[bridgeId];
        return bridge->data.id;
    }
    std::string getBridgeTransportId(std::string &bridgeId) {
        auto  bridge = this->_bridges[bridgeId];
        return bridge->data.id;
    }
    std::shared_ptr<Transport> getBridgeTransport(std::string &bridgeId) {
        auto  bridge = this->_bridges[bridgeId];
        return bridge->data.transports[bridge->data.id];
    }
   /**
     * Create a mediasoup Transport associated to a Broadcaster. It can be a
     * PlainTransport or a WebRtctransport->
     *
     * @async
     *
     * @type {String} bridgeId
     * @type {String} type - Can be "plain" (PlainTransport) or "webrtc"
     *   (WebRtcTransport).
     * @type {Boolean} [rtcpMux=false] - Just for PlainTransport, use RTCP mux.
     * @type {Boolean} [comedia=true] - Just for PlainTransport, enable remote IP:port
     *   autodetection.
     * @type {Object} [sctpCapabilities] - SCTP capabilities
     */
    json createBridgeTransport(
        
            std::string  bridgeId,
            std::string type,
            bool rtcpMux,// = false,
            bool comedia,// = true,
            SctpCapabilities &sctpCapabilities
        )
    {
        auto  bridge = this->_bridges[bridgeId];

        if (!bridge)
            MS_THROW_lOG("bridge with id bridgeId=%s does not exist",bridgeId.c_str());

        if (true)
        {
            if(type == "webrtc")
            {
                json webRtcTransportOptions =
                {
                    config.mediasoup.webRtcTransportOptions,
                    {"enableSctp"     , (sctpCapabilities.numStreams.OS != 0)},
                        {"numSctpStreams" , sctpCapabilities.numStreams},
                    {"producing" , true},
                    {"consuming" , true}
                };
                WebRtcTransportOptions options = webRtcTransportOptions;
                auto transport =  this->_mediasoupRouter->createWebRtcTransport(options);

                // Store it.
                bridge->data.transports[transport->id()]= transport;

                return {
                    {"id"             , transport->id()},
                    {"iceParameters"  , transport->iceParameters()},
                    {"iceCandidates"  , transport->iceCandidates()},
                    {"dtlsParameters" , transport->dtlsParameters()},
                    {"sctpParameters" , transport->sctpParameters()}
                };
            }else
            if(type == "plain")
            {
                json plainTransportOptions =
                {
                    config.mediasoup.plainTransportOptions,
                    {"rtcpMux" , rtcpMux},
                    {"comedia" , comedia}
                };
                PlainTransportOptions options = plainTransportOptions;
                auto transport =  this->_mediasoupRouter->createPlainTransport(options);

                // Store it.
                bridge->data.transports[transport->id()]= transport;

                return {
                    {"id"       , transport->id()},
                    {"ip"       , transport->tuple().localIp},
                    {"port"     , transport->tuple().localPort},
                    {"rtcpPort" , transport->rtcpTuple().localPort}
                };
            } else
            {
                MS_THROW_lOG("invalid type");
            }
        }
    }
    std::vector<std::shared_ptr<Producer>> getProducersFromBridge(std::string &bridgeId) {
        auto  bridge = this->_bridges[bridgeId];
        if (!bridge)
            MS_THROW_lOG("bridge with id bridgeId=%s does not exist",bridgeId.c_str());
        //return bridge->data.producers;
        std::vector<std::shared_ptr<Producer>> producers;
        for (auto &kv : bridge->data.producers)
        {
            auto &producer = kv.second;
            producers.push_back(producer);
        }
        return producers;
    }
/**
     * Create a mediasoup Producer associated to a Broadcaster.
     *
     * @async
     *
     * @type {String} bridgeId
     * @type {String} transportId
     * @type {String} kind - "audio" or "video" kind for the producer->
     * @type {RTCRtpParameters} rtpParameters - RTP parameters for the producer->
     */
    json createBridgeProducer(
        //{
            std::string &bridgeId,
            std::string &transportId,
            std::string &kind,
            RtpParameters &rtpParameters
        //}
    )
    {
        auto  bridge = this->_bridges[bridgeId];

        if (!bridge)
            MS_THROW_lOG("bridge with id bridgeId=%s does not exist",bridgeId.c_str());

        auto transport = bridge->data.transports[transportId];

        if (!transport)
            MS_THROW_lOG("transport with id transportId=%s does not exist",transportId.c_str());

        ProducerOptions options;// = {
            options.kind = kind;
            options.rtpParameters = rtpParameters;
        //};
        auto producer =
             transport->produce(options);

        // Store it.
        bridge->data.producers[producer->id()]= producer;

        // Set Producer events.
        // producer->on("score",[&]( const event::event_t<json>& e ) //, (score) =>
        // {
        //     MS_lOGD(
        //         "broadcaster producer "score" event [producerId:%s, score:%o]",
        //         producer->id(), score);
        // });

        producer->on("videoorientationchange",[&]( int videoOrientation ) //, (videoOrientation) =>
        {
            //auto videoOrientation = e.value().get<int>();
            MS_lOGD(
                "broadcaster producer videoorientationchange event [producerId:%s, videoOrientation:%o]",
                producer->id().c_str(), videoOrientation);
        });

        // Optimization: Create a server-side Consumer for each Peer.
        for (auto  peer :this->_getJoinedPeers())
        {
            this->_createConsumer(
                
                    /*consumerPeer :*/ peer,
                    /*producerPeer :*/ bridge,
                    producer
                );
        
        }

        // Add into the audioLevelObserver.
        if (producer->kind() == "audio")
        {
            json options = {
                {"producerId",producer->id()}
            };
            this->_audioLevelObserver->addProducer(options);
        }
        
        return {
      {"id", producer->id()}
    };
    }

    /**
     * Create a mediasoup Consumer associated to a Broadcaster.
     *
     * @async
     *
     * @type {String} bridgeId
     * @type {String} transportId
     * @type {String} producerId
     */
    json createBridgeConsumer(
        //{
            std::string &bridgeId,
            std::string &transportId,
            std::string &producerId
        //}
    )
    {
        auto  bridge = this->_bridges[bridgeId];

        if (!bridge)
            MS_THROW_lOG("bridge with id bridgeId=%s does not exist",bridgeId.c_str());

        if (!bridge->data.rtpCapabilities)
            MS_THROW_lOG("bridge does not have rtpCapabilities");

        auto transport = bridge->data.transports[transportId];

        if (!transport)
            MS_THROW_lOG("transport with id transportId=%s does not exist",transportId.c_str());
        ConsumerOptions options;
        options.producerId = producerId;
        options.rtpCapabilities = bridge->data.rtpCapabilities;
        auto  consumer =  transport->consume(options);

        // Store it.
        bridge->data.consumers[consumer->id()]=consumer;

        // Set Consumer events.
        consumer->on("transportclose",[&]( const event::event_t<json>& e )
        {
            // Remove from its map.
            bridge->data.consumers.erase(consumer->id());
        });

        consumer->on("producerclose",[&]( const event::event_t<json>& e )
        {
            // Remove from its map.
            bridge->data.consumers.erase(consumer->id());
        });
//json j = {{"foo", 1}, {"bar", 2}};
        return {
            {"id"         , consumer->id()},
            {"producerId" , producerId},
            {"kind"       , consumer->kind()},
            {"rtpParameters", consumer->rtpParameters()},
            {"type"          , consumer->type()}
        };
    }
/**
     * Connect a Bridge mediasoup WebRtctransport->
     *
     * @async
     *
     * @type {String} bridgeId
     * @type {String} transportId
     * @type {RTCDtlsParameters} dtlsParameters - Remote DTLS parameters.
     */
    void connectBridgeTransport(
        //{
                                std::string &bridgeId,
                                std::string &transportId,
                                DtlsParameters &dtlsParameters
        //}
    )
    {
        auto  bridge = this->_bridges[bridgeId];

        if (!bridge)
            MS_THROW_lOG("bridge with id bridgeId=%s does not exist",bridgeId.c_str());

        auto transport = bridge->data.transports[transportId];

        if (!transport)
            MS_THROW_lOG("transport with id transportId=%s does not exist",transportId.c_str());

        /*
        if (transport->constructor.name != "WebRtcTransport")
        {
            MS_THROW_lOG(
                "transport with id transportId=%s is not a WebRtcTransport",transportId.c_str());
        }*/
        json options = { {"dtlsParameters",dtlsParameters} };
        transport->connect(options);
    }
	void _handleAudioLevelObserver()
	{
		this->_audioLevelObserver->on("volumes",[&]( int volumes,std::shared_ptr<Producer> producer,int volume) //, (volumes) =>
		{
      // auto volumes = e.value().get<int>();
			//const { producer, volume } = volumes[0];
     // auto & producer = volumes[0]["producer"];
     // auto & volume = volumes[0]["volume"];
			// MS_lOGD(
			// 	"audioLevelObserver "volumes" event [producerId:%s, volume:%s]",
			// 	producer->id(), volume);

			// Notify all Peers.
			for (auto  peer :this->_getJoinedPeers())
			{
				peer->notify(
					"activeSpeaker",
					{
						{"peerId" , producer->appData()["peerId"]},
						{"volume" , volume}
					});
			}
		});

		this->_audioLevelObserver->on("silence",[&]( const event::event_t<json>& e )
		{
			// MS_lOGD("audioLevelObserver "silence" event");

			// Notify all Peers.
			for (auto  peer :this->_getJoinedPeers())
			{
				peer->notify("activeSpeaker", {{ "peerId", "" }});
			}
		});
	}

	/**
	 * Handle protoo requests from browsers.
	 *
	 * @async
	 */

	void _handleProtooRequest( std::shared_ptr<Peer> &peer, json &request, std::function<void(json data)> const& accept, std::function<void(int errorCode, std::string errorReason)> const& reject)
	{
        std::string method = request["method"];
		//switch (request.method)
        MS_lOGD("_handleProtooRequest request data = %s",request["data"].dump().c_str());
        if(true)
		{
			if(method == "getRouterRtpCapabilities")
			{
			    json rtpCapabilities = this->_mediasoupRouter->rtpCapabilities();
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
				// 	...this->_getJoinedPeers(),
				// 	...this->_broadcasters,
				// 	...this->_bridges
				// ];
                std::unordered_map<std::string,std::shared_ptr<Peer>> joinedPeers;
                std::unordered_map<std::string, shared_ptr<Peer>> joinPeers = this->_getJoinedPeersMap();
                joinedPeers.insert(joinPeers.begin(), joinPeers.end());
                joinedPeers.insert(this->_broadcasters.begin(), this->_broadcasters.end());
                joinedPeers.insert(this->_bridges.begin(), this->_bridges.end());

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
				// 	.filter((joinedPeer) => joinedPeer->id != peer->id)
				// 	.map((joinedPeer) => ({
				// 		id          : joinedPeer->id(),
				// 		displayName : joinedPeer->data.displayName,
				// 		device      : joinedPeer->data.device
				// 	}));

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
				// 	forceTcp,
				// 	producing,
				// 	consuming,
				// 	sctpCapabilities
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
				// 	...config.mediasoup.webRtcTransportOptions,
				// 	enableSctp     : Boolean(sctpCapabilities),
				// 	numSctpStreams : (sctpCapabilities || {}).numStreams,
				// 	appData        : { producing, consuming }
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
					// 	"producer "score" event [producerId:%s, score:%o]",
					// 	producer->id(), score);

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
				// 	[
				// 		...this->_getJoinedPeers(peer),
				// 		...this->_broadcasters
				// 	];
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
					//	this->sendSdpOfferToBridge(otherPeer->id(),peer->id());
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
				// 	transportid(),
				// 	sctpStreamParameters,
				// 	label,
				// 	protocol,
				// 	appData
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
//						this->_bot.handlePeerDataProducer(
//							{
//								dataProducerId : dataProducer->id(),
//								peer
//							});

					//	//break;
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

//				try
//				{
//					 throttle.start(
//						{
//							up   : uplink || DefaultUplink,
//							down : downlink || DefaultDownlink,
//							rtt  : rtt || DefaultRtt
//						});
//
//					MS_lOGW(
//						"network throttle set [uplink:%s, downlink:%s, rtt:%s]",
//						uplink || DefaultUplink,
//						downlink || DefaultDownlink,
//						rtt || DefaultRtt);
//
//				accept(json({}));
//				}
//				catch (const char *error)
//				{
//					MS_lOGE("network throttle apply failed: %s", error);
//
//					reject(500, error.toString());
//				}

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

//				try
//				{
//					 throttle.stop({});
//
//					MS_lOGW("network throttle stopped");
//
//					accept(json({}));
//				}
//				catch (const char *error)
//				{
//					MS_lOGE("network throttle stop failed: %s", error);
//
//					reject(500, error.toString());
//				}

				//break;
			}else
			{
				MS_lOGE("unknown request.method %s", method.c_str());

				reject(500, "unknown request.method request.method=%s");
			}
		}
	}

	/**
	 * Helper to get the list of joined protoo peers.
	 */
	std::vector<shared_ptr<Peer> > _getJoinedPeers(std::shared_ptr<Peer> excludePeer = nullptr)
	{
        std::vector<std::shared_ptr<Peer> > peerlist;
        for (const auto& kv : this->_peers)
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
		//	.filter((peer) => peer->data.joined && peer != excludePeer);
	}
	/**
	 * Helper to get the list of joined protoo peers.
	 */
    std::unordered_map<std::string,shared_ptr<Peer>> _getJoinedPeersMap(std::shared_ptr<Peer> excludePeer = nullptr)
	{
       std::unordered_map<std::string,shared_ptr<Peer> > peerMap;
        for (const auto& kv : this->_peers)
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
		//	.filter((peer) => peer->data.joined && peer != excludePeer);
	}

	/**
	 * Creates a mediasoup Consumer for the given mediasoup producer->
	 *
	 * @async
	 */
	void _createConsumer(std::shared_ptr<Peer> &consumerPeer, std::shared_ptr<Peer> &producerPeer, std::shared_ptr<Producer>  &producer )
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
			!this->_mediasoupRouter->canConsume(
					producer->id(),
                    rtpCapabilities
				)
		)
		{
            MS_lOGW("_createConsumer() | Transport canConsume = false return");
			return;
		}

		// Must take the Transport the remote Peer is using for consuming.
//		auto transport = Array.from(consumerPeer.data.transports)
//			.find((t) => t.appData.consuming);

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
		consumer->on("transportclose",[&]( )
		{
			// Remove from its map.
			consumerPeer->data.consumers.erase(consumer->id());
		});

		consumer->on("producerclose",[&](  )
		{
			// Remove from its map.
			consumerPeer->data.consumers.erase(consumer->id());

            consumerPeer->notify("consumerClosed", { {"consumerId", consumer->id()} });
		});

		consumer->on("producerpause",[&]()
		{
            consumerPeer->notify("consumerPaused", { {"consumerId", consumer->id()} });
		});

		consumer->on("producerresume",[&](  )
		{
            consumerPeer->notify("consumerResumed", { {"consumerId", consumer->id()} });
		});

		consumer->on("score", [&]( int score ) //(score) =>
		{
           
			// MS_lOGD(
			// 	"consumer "score" event [consumerId:%s, score:%o]",
			// 	consumer->id(), score);
			
            consumerPeer->notify("consumerScore", {
              {"consumerId", consumer->id()},
              {"score",score}
              });
			

		});

		consumer->on("layerschange",[&]( json layers ) // (layers) =>
		{
      
			consumerPeer->notify(
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

		consumer->on("trace",[&]( ConsumerTraceEventData & trace ) // (trace) =>
		{
            
			MS_lOGD(
				"consumer trace event [producerId:%s, trace.type:%s, trace]",
				consumer->id().c_str(), trace.type.c_str());
		});

		// Send a protoo request to the remote Peer with Consumer parameters.
		if(!consumerPeer->isHasRequest) {
			// consumer->resume();
			//return;
		}
		try
		{

			 consumerPeer->request(
				"newConsumer",
				{
                 {"peerId"     , producerPeer->id},
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

			if(consumerPeer->isHasNotify) {
				// consumer->resume();
				//return;
			}
           
			consumerPeer->notify(
				"consumerScore",
				{
                    {"consumerId" , consumer->id()},
                    {"score"      , consumer->score()}
				});

			 consumer->resume();
		}
		catch (const char *error)
		{
			MS_lOGW("_createConsumer() | failed:%s", error);
		}
		return;
	}
	/**
	 * Creates a mediasoup Consumer for the given mediasoup producer->
	 *
	 * @async
	 */
	void _createBridgeConsumer( std::shared_ptr<Peer> & consumerPeer, std::shared_ptr<Peer> &producerPeer, std::shared_ptr<Producer>  &producer )
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
            //!consumerPeer->data.rtpCapabilities.count() !=0  ||
            !this->_mediasoupRouter->canConsume(
                    producer->id(),
                    rtpCapabilities
                )
        )
        {
            return;
        }

        // Must take the Transport the remote Peer is using for consuming.
//        auto transport = Array.from(consumerPeer.data.transports)
//            .find((t) => t.appData.consuming);

        std::shared_ptr<Transport> transport;
        for(auto &kv : consumerPeer->data.transports)
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
        }
        catch (const char *error)
        {
            MS_lOGW("_createConsumer() | transport->consume():%s", error);

            return;
        }

        // Store the Consumer into the protoo consumerPeer data Object.
        consumerPeer->data.consumers[consumer->id()] =  consumer;

        // Set Consumer events.
        consumer->on("transportclose",[&]( )
        {
            // Remove from its map.
            consumerPeer->data.consumers.erase(consumer->id());
        });

        consumer->on("producerclose",[&](  )
        {
            // Remove from its map.
            consumerPeer->data.consumers.erase(consumer->id());

            consumerPeer->notify("consumerClosed", { {"consumerId", consumer->id()} });
        });

        consumer->on("producerpause",[&]()
        {
            consumerPeer->notify("consumerPaused", { {"consumerId", consumer->id()} });
        });

        consumer->on("producerresume",[&](  )
        {
            consumerPeer->notify("consumerResumed", { {"consumerId", consumer->id()} });
        });

        consumer->on("score", [&]( int score ) //(score) =>
        {
           
            // MS_lOGD(
            //     "consumer "score" event [consumerId:%s, score:%o]",
            //     consumer->id(), score);
            
            consumerPeer->notify("consumerScore", {
              {"consumerId", consumer->id()},
              {"score",score}
              });
            

        });

        consumer->on("layerschange",[&]( json layers ) // (layers) =>
        {
      
            consumerPeer->notify(
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

        consumer->on("trace",[&]( ConsumerTraceEventData & trace ) // (trace) =>
        {
            
            MS_lOGD(
                "consumer trace event [producerId:%s, trace.type:%s, trace]",
                consumer->id().c_str(), trace.type.c_str());
        });

        // Send a protoo request to the remote Peer with Consumer parameters.
        if(!consumerPeer->isHasRequest) {
             consumer->resume();
            return;
        }
        try
        {
             consumerPeer->request(
                "newConsumer",
                {
                 {"peerId"     , producerPeer->id},
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

            if(consumerPeer->isHasNotify) {
                 consumer->resume();
                return;
            }
           
//            consumerPeer->notify(
//                "consumerScore",
//                {
//                    {"consumerId" , consumer->id()},
//                    {"score"      , consumer->score()}
//                });

             consumer->resume();
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
	void _createDataConsumer(
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
//		auto transport = Array.from(dataConsumerPeer.data.transports)
//			.find((t) => t.appData.consuming);
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
        dataConsumer->on("transportclose",[&](  )
		{
			// Remove from its map.
			dataConsumerPeer->data.dataConsumers.erase(dataConsumer->id());
		});

        dataConsumer->on("dataproducerclose",[&]( )
		{
			// Remove from its map.
			dataConsumerPeer->data.dataConsumers.erase(dataConsumer->id());

			dataConsumerPeer->notify(
                                    "dataConsumerClosed", { {"dataConsumerId", dataConsumer->id()} });
		});

		// Send a protoo request to the remote Peer with Consumer parameters.
		try
		{
			 dataConsumerPeer->request(
				"newDataConsumer",
				{
					// This is null for bot dataProducer->
					{"peerId"            , dataProducerPeer->id },
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
	void sendSdpOfferToBridge(std::shared_ptr<Peer> &bridge,std::string peerId) {
		std::vector<std::shared_ptr<Consumer>> newConsumers;
		auto bridgeId = bridge->id;
		//auto  bridge = this->_bridges[bridgeId];
		for (auto  &kv : bridge->data.consumers) {
            auto &consumer = kv.second;
            newConsumers.push_back(consumer);
		}
		//MS_lOGD("createWebrtcSdpOffer android transportId=%s",transport->id().c_str());
		auto btransport = this->getBridgeTransport(bridgeId);
		MS_lOGD("createWebrtcSdpOffer bridge transportId=%s",btransport->id().c_str());
		auto sdpOffer = mediasoupclient::Sdp::Offer::createWebrtcSdpOffer(btransport,newConsumers);
		json payload = {
			{"type","offer"},
			{"sdp",sdpOffer}
		};
		MS_lOGI("sdOffer=%s",sdpOffer.dump().c_str());
        Base64 base;
        auto msgpayload = base.Encode((const unsigned char*)payload.dump().c_str(),payload.dump().length());
		//auto b = new Buffer.from(JSON.stringify(payload));
		//auto msgpayload = b.toString("base64");
		json jsonmsg = {
			{"senderClientId",peerId},
			{"messageType","SDP_OFFER" },
			{"RecipientClientId",bridgeId },
			{"messagePayload",msgpayload },
			{"statusResponse",{
          {"correlationId","1"},
          {"errorType","ok"},
          {"statusCode",0},
          {"description","ok"}
        }
			}
		};
		MS_lOGI("websocket send msg=%s",jsonmsg.dump().c_str());
		//this->websocket.send(jsonmsg);
	}
};

