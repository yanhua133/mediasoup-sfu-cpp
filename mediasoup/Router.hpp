#pragma once
#include <string>
#include <vector>
//#include <boost/lockfree/queue.hpp>
#include <stdexcept>
#include "Log.hpp"
#include "ChannelAgent.hpp"
#include "PayloadChannelAgent.hpp"
#include "Transport.hpp"
#include "WebRtcTransport.hpp"
#include "PlainTransport.hpp"
#include "PipeTransport.hpp"
#include "DirectTransport.hpp"
#include "Producer.hpp"
#include "Consumer.hpp"
#include "DataProducer.hpp"
#include "DataConsumer.hpp"
#include "RtpParameters.hpp"
#include "SctpParameters.hpp"
#include "RtpObserver.hpp"
#include "EnhancedEventEmitter.hpp"
#include "AudioLevelObserver.hpp"
#include "Promise.hpp"
using json = nlohmann::json;
using namespace mediasoup;

using namespace uuid;
namespace mediasoup
{
  
class Router;

struct RouterOptions
{
	/**
	 * Router media codecs.
	 */
	std::vector<RtpCodecCapability> mediaCodecs;

	/**
	 * Custom application data.
	 */
	json _appData;
};

struct PipeToRouterOptions
{
	/**
	 * The id of the Producer to consume.
	 */
	std::string producerId;

	/**
	 * The id of the DataProducer to consume.
	 */
	std::string dataProducerId;

	/**
	 * TarRouter instance.
	 */
    std::shared_ptr<Router> router;

	/**
	 * IP used in the PipeTransport pair. Default '127.0.0.1'.
	 */
	std::string listenIp; //?: TransportListenIp | string;

	/**
	 * Create a SCTP association. Default false.
	 */
	bool enableSctp;

	/**
	 * SCTP streams number.
	 */
	int numSctpStreams ;

	/**
	 * Enable RTX and NACK for RTP retransmission.
	 */
	bool enableRtx;

	/**
	 * Enable SRTP.
	 */
	bool enableSrtp;
};

struct PipeToRouterResult
{
	/**
	 * The Consumer created in the current Router.
	 */
    std::shared_ptr<Consumer> pipeConsumer;

	/**
	 * The Producer created in the tarRouter.
	 */
    std::shared_ptr<Producer> pipeProducer;

	/**
	 * The DataConsumer created in the current Router.
	 */
    std::shared_ptr<DataConsumer> pipeDataConsumer;

	/**
	 * The DataProducer created in the tarRouter.
	 */
    std::shared_ptr<DataProducer> pipeDataProducer;
};

//const logger = new Logger('Router");

class Router   : public EnhancedEventEmitter, public std::enable_shared_from_this<Router>
{
public:
	// Internal data.
    /*
	struct  internal:
	{
		std::string routerId;
	};*/
    json _internal;
	// Router data.
    /*
	struct data:
	{
		RtpCapabilities rtpCapabilities;
	};*/
    json _data;
	// Channel instance.
	std::shared_ptr<ChannelAgent> _channel;//: Channel;

	// PayloadChannel instance.
	std::shared_ptr<PayloadChannelAgent> _payloadChannel;//: PayloadChannelAgent;

	// Closed flag.
	bool _closed = false;

	// Custom app data.
	 json _appData;
    // Transports map.

    std::unordered_map<std::string,std::shared_ptr<Transport> > _transports;

    // Producers map.
    std::unordered_map<std::string,std::shared_ptr<Producer> > _producers;

    // RtpObservers map.
    std::unordered_map<std::string,std::shared_ptr<RtpObserver> > _rtpObservers;

    // DataProducers map.
    std::unordered_map<std::string,std::shared_ptr<DataProducer> > _dataProducers;
    
    // Router to PipeTransport map.
    std::unordered_map<std::string,std::vector<std::shared_ptr<PipeTransport> > > _mapRouterPipeTransports;
    

	// AwaitQueue instance to make pipeToRouter tasks happen sequentially.
//	 _pipeToRouterQueue =
//		new AwaitQueue({ ClosedErrorClass: InvalidStateError });
    //typedef  std::function<void(std::shared_ptr<PipeTransport>,std::shared_ptr<PipeTransport>)> funcQueueItem;
    //boost::lockfree::queue<int> _pipeToRouterQueue;
	// Observer instance.
    EnhancedEventEmitter *_observer = new EnhancedEventEmitter();

  
   
	/**
	 * @private
	 * @emits workerclose
	 * @emits @close
	 */
	Router(
	    json& internal,
			json& data,
			std::shared_ptr<ChannelAgent> &channel,
			std::shared_ptr<PayloadChannelAgent> &payloadChannel,
			json &appData
	)
	{
		////super();

		MS_lOGD("Router()");

		this->_internal = internal;
		this->_data = data;
		this->_channel = channel;
		this->_payloadChannel = payloadChannel;
		this->_appData = appData;
        
        this->getProducerByIdFunc = [&](std::string &producerId) {
              MS_lOGD("getProducerByIdFunc(producerId=%s) ",producerId.c_str());
              return this->getProducerById(producerId);
        };
        this->getDataProducerByIdFunc = [&](std::string &dataProducerId) {
            MS_lOGD("getDataProducerByIdFunc(dataProducerId=%s) ",dataProducerId.c_str());
              return this->getDataProducerById(dataProducerId);
        };

        this->getRouterRtpCapabilitiesFunc = [&]() {
            MS_lOGD("getRouterRtpCapabilitiesFunc");
            return this->getRouterRtpCapabilities();
        };
//        std::string bbb= "aaa";
//        auto p = this->getProducerByIdFunc(bbb);
//        int i=0;
//        i=1;

	}

	/**
	 * Router id.
	 */
	std::string id()
	{
		return this->_internal["routerId"];
	}

	/**
	 * Whether the Router is closed.
	 */
	bool closed()
	{
		return this->_closed;
	}

	/**
	 * RTC capabilities of the Router.
	 */
	RtpCapabilities rtpCapabilities()
	{
		return this->_data["rtpCapabilities"];
	}

	/**
	 * App custom data.
	 */
	json appData()//: any
	{
		return this->_appData;
	}

	/**
	 * Invalid setter.
	 */
	void setAppData(json appData) // eslint-disable-line no-unused-vars
	{
		MS_THROW_lOG("cannot override appData object");
	}

	/**
	 * Observer.
	 *
	 * @emits close
	 * @emits newtransport - (transport: Transport)
	 * @emits newrtpobserver - (rtpObserver: RtpObserver)
	 */
	EnhancedEventEmitter * observer()
	{
		return this->_observer;
	}

	/**
	 * Close the Router.
	 */
	void close()
	{
		if (this->_closed)
			return;

		MS_lOGD("close()");

		this->_closed = true;

		this->_channel->request("router.close", this->_internal);

		// Close every Transport.
        for (const auto& kv : this->_transports)
        {
            auto transportId        = kv.first;
            auto &transport   = kv.second;
		
			transport->routerClosed();
		}
		this->_transports.clear();

		// Clear the Producers map.
		this->_producers.clear();

		// Close every RtpObserver.
        for (const auto& kv : this->_rtpObservers)
        {
           // auto transportId        = kv.first;
            auto &rtpObserver   = kv.second;
		
			rtpObserver->routerClosed();
		}
		this->_rtpObservers.clear();

		// Clear the DataProducers map.
		this->_dataProducers.clear();

		// Clear map of Router/PipeTransports.
		this->_mapRouterPipeTransports.clear();

		// Close the pipeToRouter AwaitQueue instance.
		//this->_pipeToRouterQueue.close();

		this->emit("@close");

		// Emit observer event.
		this->_observer->safeEmit("close");
	}

	/**
	 * Worker was closed.
	 *
	 * @private
	 */
	void workerClosed()
	{
		if (this->_closed)
			return;

		MS_lOGD("workerClosed()");

		this->_closed = true;

		// Close every Transport.
        for (const auto& kv : this->_transports)
        {
           // auto transportId        = kv.first;
            auto &transport   = kv.second;
		
			transport->routerClosed();
		}
		this->_transports.clear();

		// Clear the Producers map.
		this->_producers.clear();

		// Close every RtpObserver.
        for (const auto& kv : this->_rtpObservers)
        {
           // auto transportId        = kv.first;
            auto &rtpObserver   = kv.second;
    
			rtpObserver->routerClosed();
		}
		this->_rtpObservers.clear();

		// Clear the DataProducers map.
		this->_dataProducers.clear();

		// Clear map of Router/PipeTransports.
		this->_mapRouterPipeTransports.clear();

		this->safeEmit("workerclose");

		// Emit observer event.
		this->_observer->safeEmit("close");
	}

	/**
	 * Dump Router.
	 */
	json dump() //Promise<any>
	{
		MS_lOGD("dump()");

		return this->_channel->request("router.dump", this->_internal);
	}
    
    RtpCapabilities getRouterRtpCapabilities()
    {
        return this->_data["rtpCapabilities"];
    }
    
    std::shared_ptr<Producer> &getProducerById(std::string &producerId)
    {
        return this->_producers[producerId];
    }
    
    std::shared_ptr<DataProducer> &getDataProducerById(std::string &dataProducerId)
    {
        return this->_dataProducers[dataProducerId];
    }
    std::function<std::shared_ptr<Producer>(std::string &producerId)> getProducerByIdFunc;
    std::function<std::shared_ptr<DataProducer>(std::string &dataProducerId)> getDataProducerByIdFunc;
    std::function<RtpCapabilities()> getRouterRtpCapabilitiesFunc;

	/**
	 * Create a WebRtcTransport.
	 */
    std::shared_ptr<WebRtcTransport> createWebRtcTransport(
        WebRtcTransportOptions & options
	)//: Promise<WebRtcTransport>
	{
		MS_lOGD("createWebRtcTransport()");
        json listenIps = options.listenIps;
        bool enableUdp = options.enableUdp;
        bool enableTcp = options.enableTcp;
        bool preferUdp = options.preferUdp;
        bool preferTcp = options.preferTcp;
        MS_lOGD("createWebRtcTransport() listenIps = %s",listenIps.dump(4).c_str());
        int initialAvailableOutgoingBitrate = options.initialAvailableOutgoingBitrate;
        bool enableSctp = options.enableSctp;
        NumSctpStreams numSctpStreams =options.numSctpStreams; ;//= { OS: 1024, MIS: 1024 },
        int maxSctpMessageSize = options.maxSctpMessageSize;
        int sctpSendBufferSize = options.sctpSendBufferSize;
        json appData = options.appData;
//		if (!Array.isArray(listenIps))
//			MS_THROW_lOG("missing listenIps");
//		else if (appData && typeof appData !== 'object')
//			MS_THROW_lOG("if given, appData must be an object");

//		listenIps = listenIps.map((listenIp) =>
//		{
//			if (typeof listenIp === 'string' && listenIp)
//			{
//				return { ip: listenIp };
//			}
//			else if (typeof listenIp === 'object')
//			{
//				return {
//					ip          : listenIp.ip,
//					announcedIp : listenIp.announcedIp || undefined
//				};
//			}
//			else
//			{
//				MS_THROW_lOG("wrong listenIp");
//			}
//		});
     //   listenIps();

		//const internal = { ...this->_internal, transportId: uuidv4() };
        json internal = this->_internal;
		internal["transportId"] = uuidv4();
		const json reqData = {
            {"listenIps",listenIps},
            {"enableUdp",enableUdp},
            {"enableTcp",enableTcp},
            {"preferUdp",preferUdp},
            {"preferTcp",preferTcp},
            {"initialAvailableOutgoingBitrate",initialAvailableOutgoingBitrate},
            {"enableSctp",enableSctp},
            {"numSctpStreams",numSctpStreams},
            {"maxSctpMessageSize",maxSctpMessageSize},
            {"sctpSendBufferSize",sctpSendBufferSize},
            {"isDataChannel" , true}
		
        };

		json data =
			this->_channel->request("router.createWebRtcTransport", internal, reqData);
		MS_lOGD("createWebRtcTransport router.createWebRtcTransport data=%s",data.dump(4).c_str());
        TransportParams params;
        params.internal = internal;
        params.data = data;
        params.channel = this->_channel;
        params.payloadChannel = this->_payloadChannel;
        params.appData = appData;
        params.getRouterRtpCapabilities = this->getRouterRtpCapabilitiesFunc;
        params.getProducerById = this->getProducerByIdFunc;
        params.getDataProducerById = this->getDataProducerByIdFunc;

        std::shared_ptr<WebRtcTransport> transport = std::make_shared<WebRtcTransport>(
             params
        );
		transport->handleWorkerNotifications();
        //{
//            internal,
//            data1,
//            this->_channel,
//            this->_payloadChannel,
//            appData,
//            getRouterRtpCapabilities,// : (): RtpCapabilities => this->_data.rtpCapabilities,
//            getProducerById ,//         : (producerId): Producer | undefined => (
//            //    this->_producers.get(producerId)
//            //),
//            getDataProducerById,// : (dataProducerId): DataProducer | undefined => (
//            //    this->_dataProducers.get(dataProducerId)
//            //)
        //}
		this->_transports[transport->id()] = transport;
		//transport.on('@close', () => this->_transports.delete(transport.id));
        transport->on("@close",[self = shared_from_this(), transport](  )
        {
            self->_transports.erase(transport->id());

        });
        
        //transport.on('@newproducer', (producer: Producer) => this->_producers.set(producer.id, producer));
        transport->on("@newproducer",[self = shared_from_this()]( std::shared_ptr<Producer> &producer)
        {
            self->_producers[producer->id()] = producer;

        });
            //transport.on('@producerclose', (producer: Producer) => this->_producers.delete(producer.id));
		transport->on("@producerclose", [self = shared_from_this()](std::shared_ptr<Producer>& producer)
			{
				self->_producers.erase(producer->id());
			});
            //transport.on('@newdataproducer', (dataProducer: DataProducer) => (
            //	this->_dataProducers.set(dataProducer.id, dataProducer)
            //));
        transport->on("@newdataproducer",[self = shared_from_this()](  std::shared_ptr<DataProducer> &dataProducer)
        {
            self->_dataProducers[dataProducer->id()] = dataProducer;
            
        });
            //transport.on('@dataproducerclose', (dataProducer: DataProducer) => (
            //	this->_dataProducers.delete(dataProducer.id)
            //));
        transport->on("@dataproducerclose",[self = shared_from_this()](std::string id)
        {
            self->_dataProducers.erase(id);            

        });

		// Emit observer event.
		this->_observer->safeEmit("newtransport", transport);

		return transport;
	}

	/**
	 * Create a PlainTransport.
	 */
  std::shared_ptr<PlainTransport>  createPlainTransport(
		//{
          PlainTransportOptions & options
		//}: PlainTransportOptions
	)//: Promise<PlainTransport>
	{
		MS_lOGD("createPlainTransport()");
        json listenIp = options.listenIp;
        bool rtcpMux = options.rtcpMux;
        bool comedia = options.comedia;
        bool enableSctp = options.enableSctp;
        NumSctpStreams numSctpStreams =options.numSctpStreams; ;//= { OS: 1024, MIS: 1024 },
        int maxSctpMessageSize = options.maxSctpMessageSize;
        int sctpSendBufferSize = options.sctpSendBufferSize;
        std::string srtpCryptoSuite = options.srtpCryptoSuite;
        bool enableSrtp = options.enableSrtp;
        json appData  = options._appData;
//		if (!listenIp)
//			MS_THROW_lOG("missing listenIp");
//		else if (appData && typeof appData !== 'object')
//			MS_THROW_lOG("if given, appData must be an object");

//		if (typeof listenIp === 'string' && listenIp)
//		{
//			listenIp = { ip: listenIp };
//		}
//		else if (typeof listenIp === 'object')
//		{
//			listenIp =
//			{
//				ip          : listenIp.ip,
//				announcedIp : listenIp.announcedIp || undefined
//			};
//		}
//		else
//		{
//			MS_THROW_lOG("wrong listenIp");
//		}

		//const internal = { ...this->_internal, transportId: uuidv4() };
        json internal = this->_internal;
		internal["transportId"] = uuidv4();
		const json reqData = {
            {"listenIp",listenIp},
            {"rtcpMux",rtcpMux},
            {"comedia",comedia},
            {"enableSctp",enableSctp},
            {"numSctpStreams",numSctpStreams},
            {"maxSctpMessageSize",maxSctpMessageSize},
            {"sctpSendBufferSize",sctpSendBufferSize},
            {"isDataChannel" , false},
            {"enableSrtp",enableSrtp},
            {"srtpCryptoSuite",srtpCryptoSuite}
		
    };

		json data =
			this->_channel->request("router.createPlainTransport", internal, reqData);
        TransportParams params;
        params.internal = internal;
        params.data = data;
        params.channel = this->_channel;
        params.payloadChannel = this->_payloadChannel;
        params.appData = appData;
        params.getRouterRtpCapabilities = this->getRouterRtpCapabilitiesFunc;
        params.getProducerById = this->getProducerByIdFunc;
        params.getDataProducerById = this->getDataProducerByIdFunc;
   
        std::shared_ptr<PlainTransport> transport = std::make_shared<PlainTransport>(params);
		transport->handleWorkerNotifications();

//        internal,
//        data,
//        this->_channel,
//        this->_payloadChannel,
//        appData,
//        getRouterRtpCapabilities,// : (): RtpCapabilities => this->_data.rtpCapabilities,
//        getProducerById,//          : (producerId): Producer | undefined => (
//        //    this->_producers.get(producerId)
//        //),
//        getDataProducerById,// : (dataProducerId): DataProducer | undefined => (
//        //    this->_dataProducers.get(dataProducerId)
//        //)
		this->_transports[transport->id()] = transport;
        //transport.on('@close', () => this->_transports.delete(transport.id));
        transport->on("@close",[self = shared_from_this(), transport](  )
        {
            self->_transports.erase(transport->id());

        });
        
        //transport.on('@newproducer', (producer: Producer) => this->_producers.set(producer.id, producer));
        transport->on("@newproducer",[self = shared_from_this()]( std::shared_ptr<Producer> &producer)
        {
            self->_producers[producer->id()] = producer;
        });
            //transport.on('@producerclose', (producer: Producer) => this->_producers.delete(producer.id));
        transport->on("@producerclose",[self = shared_from_this()](std::shared_ptr<Producer>& producer)
        {
            self->_producers.erase(producer->id());
        });
            //transport.on('@newdataproducer', (dataProducer: DataProducer) => (
            //    this->_dataProducers.set(dataProducer.id, dataProducer)
            //));
        transport->on("@newdataproducer",[self = shared_from_this()](std::shared_ptr<DataProducer> &dataProducer)
        {
            self->_dataProducers[dataProducer->id()] = dataProducer;            
        });
            //transport.on('@dataproducerclose', (dataProducer: DataProducer) => (
            //    this->_dataProducers.delete(dataProducer.id)
            //));
        transport->on("@dataproducerclose",[self = shared_from_this()](std::string id)
        {
            self->_dataProducers.erase(id);
        });


		// Emit observer event.
		this->_observer->safeEmit("newtransport", transport);

		return transport;
	}

	/**
	 * Create a PipeTransport.
	 */
    std::shared_ptr<PipeTransport> createPipeTransport(
        PipeTransportOptions & options
	)//: Promise<PipeTransport>
	{
		MS_lOGD("createPipeTransport()");
        json listenIp = options.listenIp;
        bool enableSctp = options.enableSctp;
        NumSctpStreams numSctpStreams =options.numSctpStreams; ;//= { OS: 1024, MIS: 1024 },
        int maxSctpMessageSize = options.maxSctpMessageSize;
        int sctpSendBufferSize = options.sctpSendBufferSize;
        bool enableRtx = options.enableRtx;
        bool enableSrtp = options.enableSrtp;
        json appData = options._appData;
//		if (!listenIp)
//			MS_THROW_lOG("missing listenIp");
//		else if (appData && typeof appData !== 'object')
//			MS_THROW_lOG("if given, appData must be an object");

//		if (typeof listenIp === 'string' && listenIp)
//		{
//			listenIp = { ip: listenIp };
//		}
//		else if (typeof listenIp === 'object')
//		{
//			listenIp =
//			{
//				ip          : listenIp.ip,
//				announcedIp : listenIp.announcedIp || undefined
//			};
//		}
//		else
//		{
//			MS_THROW_lOG("wrong listenIp");
//		}

		//const internal = { ...this->_internal, transportId: uuidv4() };
        json internal = this->_internal;
		internal["transportId"] = uuidv4();
		const json reqData = {
            {"listenIp",listenIp},
            {"enableSctp",enableSctp},
            {"numSctpStreams",numSctpStreams},
            {"maxSctpMessageSize",maxSctpMessageSize},
            {"sctpSendBufferSize",sctpSendBufferSize},
            {"isDataChannel" , false},
            {"enableRtx",enableRtx},
            {"enableSrtp",enableSrtp}
    };

		json data =
			this->_channel->request("router.createPipeTransport", internal, reqData);
        TransportParams params;
        params.internal = internal;
        params.data = data;
        params.channel = this->_channel;
        params.payloadChannel = this->_payloadChannel;
        params.appData = appData;
        params.getRouterRtpCapabilities = this->getRouterRtpCapabilitiesFunc;
        params.getProducerById = this->getProducerByIdFunc;
        params.getDataProducerById = this->getDataProducerByIdFunc;
        std::shared_ptr<PipeTransport> transport = std::make_shared<PipeTransport>(params);
		transport->handleWorkerNotifications();
//        internal,
//        data,
//        this->_channel,
//        this->_payloadChannel,
//        appData,
//        getRouterRtpCapabilities,// : (): RtpCapabilities => this->_data.rtpCapabilities,
//        getProducerById,//          : (producerId): Producer | undefined => (
//        //    this->_producers.get(producerId)
//        //),
//        getDataProducerById,// : (dataProducerId): DataProducer | undefined => (
		this->_transports[transport->id()] = transport;
        //transport.on('@close', () => this->_transports.delete(transport.id));
        transport->on("@close",[self = shared_from_this(), transport](  )
        {
            self->_transports.erase(transport->id());
        });
        
        //transport.on('@newproducer', (producer: Producer) => this->_producers.set(producer.id, producer));
        transport->on("@newproducer",[self = shared_from_this()](std::shared_ptr<Producer>& producer)
		{
			self->_producers[producer->id()] = producer;
		});
            //transport.on('@producerclose', (producer: Producer) => this->_producers.delete(producer.id));
        transport->on("@producerclose",[self = shared_from_this()](std::shared_ptr<Producer>& producer)
		{
			self->_producers.erase(producer->id());
		});
            //transport.on('@newdataproducer', (dataProducer: DataProducer) => (
            //    this->_dataProducers.set(dataProducer.id, dataProducer)
            //));
        transport->on("@newdataproducer",[self = shared_from_this()](std::shared_ptr<DataProducer>& dataProducer)
		{
			self->_dataProducers[dataProducer->id()] = dataProducer;
		});
            //transport.on('@dataproducerclose', (dataProducer: DataProducer) => (
            //    this->_dataProducers.delete(dataProducer.id)
            //));
        transport->on("@dataproducerclose",[self = shared_from_this()](std::string id)
		{
			self->_dataProducers.erase(id);
		});

		// Emit observer event.
		this->_observer->safeEmit("newtransport", transport);

		return transport;
	}

	/**
	 * Create a DirectTransport.
	 */
    std::shared_ptr<DirectTransport> createDirectTransport(
          DirectTransportOptions &options
	)
	{
		MS_lOGD("createDirectTransport()");
        int maxMessageSize = options.maxMessageSize;
        json appData = options._appData;
		//const internal = { ...this->_internal, transportId: uuidv4() };
        json internal = this->_internal;
		internal["transportId"] = uuidv4();
		const json reqData = {
            { "direct", true },
            {"maxMessageSize",maxMessageSize }
        };

		json data =
			this->_channel->request("router.createDirectTransport", internal, reqData);
        TransportParams params;
        params.internal = internal;
        params.data = data;
        params.channel = this->_channel;
        params.payloadChannel = this->_payloadChannel;
        params.appData = appData;
        params.getRouterRtpCapabilities = this->getRouterRtpCapabilitiesFunc;
        params.getProducerById = this->getProducerByIdFunc;
        params.getDataProducerById = this->getDataProducerByIdFunc;
        std::shared_ptr<DirectTransport> transport = std::make_shared<DirectTransport>(params);
		transport->handleWorkerNotifications();
//        internal,
//        data,
//        this->_channel,
//        this->_payloadChannel,
//        appData,
//        getRouterRtpCapabilities,// : (): RtpCapabilities => this->_data.rtpCapabilities,
//        getProducerById,//          : (producerId): Producer | undefined => (
//        //    this->_producers.get(producerId)
//        //),
//        getDataProducerById// : (dataProducerId): DataProducer | undefined => (
		this->_transports[transport->id()] =  transport;
        //transport.on('@close', () => this->_transports.delete(transport.id));
        transport->on("@close",[self = shared_from_this(), transport]()
		{
			self->_transports.erase(transport->id());
		});
        
        //transport.on('@newproducer', (producer: Producer) => this->_producers.set(producer.id, producer));
        transport->on("@newproducer",[self = shared_from_this()](std::shared_ptr<Producer>& producer)
		{
			self->_producers[producer->id()] = producer;
		});
            //transport.on('@producerclose', (producer: Producer) => this->_producers.delete(producer.id));
        transport->on("@producerclose",[self = shared_from_this()](std::shared_ptr<Producer>& producer)
		{
			self->_producers.erase(producer->id());
		});
            //transport.on('@newdataproducer', (dataProducer: DataProducer) => (
            //    this->_dataProducers.set(dataProducer.id, dataProducer)
            //));
        transport->on("@newdataproducer",[self = shared_from_this()](std::shared_ptr<DataProducer>& dataProducer)
		{
			self->_dataProducers[dataProducer->id()] = dataProducer;
		});
            //transport.on('@dataproducerclose', (dataProducer: DataProducer) => (
            //    this->_dataProducers.delete(dataProducer.id)
            //));
        transport->on("@dataproducerclose",[self = shared_from_this()](std::string id)
		{
			self->_dataProducers.erase(id);
		});

		// Emit observer event.
		this->_observer->safeEmit("newtransport", transport);

		return transport;
	}

	/**
	 * Pipes the given Producer or DataProducer into another Router in same host.
	 */
    PipeToRouterResult *pipeToRouter(
         PipeToRouterOptions & options
	)//: Promise<PipeToRouterResult>
	{
        std::string producerId;
        std::string dataProducerId;
        //Router router;
        std::string listenIp = "127.0.0.1";
        bool enableSctp = true;
        int numSctpStreams = 0;// = { OS: 1024, MIS: 1024 },
        bool enableRtx = false;
        bool enableSrtp = false;
//		if (!producerId && !dataProducerId)
//			MS_THROW_lOG("missing producerId or dataProducerId");
//		else if (producerId && dataProducerId)
//			MS_THROW_lOG("just producerId or dataProducerId can be given");
//		else if (!router)
//			MS_THROW_lOG("Router not found");
//		else if (router === this)
//			MS_THROW_lOG("cannot use this Router as destination");

//		let producer: Producer | undefined;
//		let dataProducer: DataProducer | undefined;
//
//		if (producerId)
//		{
//			producer = this->_producers.get(producerId);
//
//			if (!producer)
//				MS_THROW_lOG("Producer not found");
//		}
//		else if (dataProducerId)
//		{
//			dataProducer = this->_dataProducers.get(dataProducerId);
//
//			if (!dataProducer)
//				MS_THROW_lOG("DataProducer not found");
//		}

		// Here we may have to create a new PipeTransport pair to connect source and
		// destination Routers. We just want to keep a PipeTransport pair for each
		// pair of Routers. Since this operation is async, it may happen that two
		// simultaneous calls to router1.pipeToRouter({ producerId: xxx, router: router2 })
		// would end up generating two pairs of PipeTranports. To prevent that, let's
		// use an queue.
/*
		let localPipeTransport: PipeTransport | undefined;
		let remotePipeTransport: PipeTransport | undefined;

		this->_pipeToRouterQueue.push(() =>
		{
			let pipeTransportPair = this->_mapRouterPipeTransports.get(router);

			if (pipeTransportPair)
			{
				localPipeTransport = pipeTransportPair[0];
				remotePipeTransport = pipeTransportPair[1];
			}
			else
			{
				try
				{
					pipeTransportPair = Promise.all(
						[
							this->createPipeTransport(
								{ listenIp, enableSctp, numSctpStreams, enableRtx, enableSrtp }),

							router.createPipeTransport(
								{ listenIp, enableSctp, numSctpStreams, enableRtx, enableSrtp })
						]);

					localPipeTransport = pipeTransportPair[0];
					remotePipeTransport = pipeTransportPair[1];

					Promise.all(
						[
							localPipeTransport.connect(
								{
									ip             : remotePipeTransport.tuple.localIp,
									port           : remotePipeTransport.tuple.localPort,
									srtpParameters : remotePipeTransport.srtpParameters
								}),

							remotePipeTransport.connect(
								{
									ip             : localPipeTransport.tuple.localIp,
									port           : localPipeTransport.tuple.localPort,
									srtpParameters : localPipeTransport.srtpParameters
								})
						]);

					localPipeTransport.observer.on('close', () =>
					{
						remotePipeTransport!.close();
						this->_mapRouterPipeTransports.delete(router);
					});

					remotePipeTransport.observer.on('close', () =>
					{
						localPipeTransport!.close();
						this->_mapRouterPipeTransports.delete(router);
					});

					this->_mapRouterPipeTransports.set(
						router, [ localPipeTransport, remotePipeTransport ]);
				}
				catch (error)
				{
					MS_lOGE(
						'pipeToRouter() | error creating PipeTransport pair:%o',
						error);

					if (localPipeTransport)
						localPipeTransport.close();

					if (remotePipeTransport)
						remotePipeTransport.close();

					throw error;
				}
			}
		});

		if (producer)
		{
			let pipeConsumer: Consumer | undefined;
			let pipeProducer: Producer | undefined;

			try
			{
				pipeConsumer = localPipeTransport!.consume(
					{
						producerId : producerId!
					});

				pipeProducer = remotePipeTransport!.produce(
					{
						id            : producer.id,
						kind          : pipeConsumer!.kind,
						rtpParameters : pipeConsumer!.rtpParameters,
						paused        : pipeConsumer!.producerPaused,
						appData       : producer.appData
					});

				// Pipe events from the pipe Consumer to the pipe Producer.
				pipeConsumer!.observer.on('close', () => pipeProducer!.close());
				pipeConsumer!.observer.on('pause', () => pipeProducer!.pause());
				pipeConsumer!.observer.on('resume', () => pipeProducer!.resume());

				// Pipe events from the pipe Producer to the pipe Consumer.
				pipeProducer.observer.on('close', () => pipeConsumer!.close());

				return { pipeConsumer, pipeProducer };
			}
			catch (error)
			{
				MS_lOGE(
					'pipeToRouter() | error creating pipe Consumer/Producer pair:%o',
					error);

				if (pipeConsumer)
					pipeConsumer.close();

				if (pipeProducer)
					pipeProducer.close();

				throw error;
			}
		}
		else if (dataProducer)
		{
			let pipeDataConsumer: DataConsumer | undefined;
			let pipeDataProducer: DataProducer | undefined;

			try
			{
				pipeDataConsumer = localPipeTransport!.consumeData(
					{
						dataProducerId : dataProducerId!
					});

				pipeDataProducer = remotePipeTransport!.produceData(
					{
						id                   : dataProducer.id,
						sctpStreamParameters : pipeDataConsumer!.sctpStreamParameters,
						label                : pipeDataConsumer!.label,
						protocol             : pipeDataConsumer!.protocol,
						appData              : dataProducer.appData
					});

				// Pipe events from the pipe DataConsumer to the pipe DataProducer.
				pipeDataConsumer!.observer.on('close', () => pipeDataProducer!.close());

				// Pipe events from the pipe DataProducer to the pipe DataConsumer.
				pipeDataProducer.observer.on('close', () => pipeDataConsumer!.close());

				return { pipeDataConsumer, pipeDataProducer };
			}
			catch (error)
			{
				MS_lOGE(
					'pipeToRouter() | error creating pipe DataConsumer/DataProducer pair:%o',
					error);

				if (pipeDataConsumer)
					pipeDataConsumer.close();

				if (pipeDataProducer)
					pipeDataProducer.close();

				throw error;
			}
		}
		else
		{
			MS_THROW_lOG("internal error");
		}
 */
	}

	/**
	 * Create an AudioLevelObserver.
	 */
    std::shared_ptr<AudioLevelObserver> createAudioLevelObserver(
          AudioLevelObserverOptions &options
	)//: Promise<AudioLevelObserver>
	{
		MS_lOGD("createAudioLevelObserver()");
        int maxEntries = 1;
        int threshold = -80;
        int interval = 1000;
        json appData;
		//if (appData && typeof appData !== 'object')
		//	MS_THROW_lOG("if given, appData must be an object");

		//const internal = { ...this->_internal, rtpObserverId: uuidv4() };
        json internal = this->_internal;
		internal["rtpObserverId"] = uuidv4();
		const json reqData = {
            {"maxEntries",maxEntries},
            {"threshold",threshold},
            {"interval",interval }
        };

		this->_channel->request("router.createAudioLevelObserver", internal, reqData);
        RtpObserverParams params;
        params.internal = internal;
        params.channel = this->_channel;
        params.payloadChannel = this->_payloadChannel;
        params.appData = appData;
        params.getProducerById = this->getProducerByIdFunc;

        std::shared_ptr<AudioLevelObserver> audioLevelObserver = std::make_shared<AudioLevelObserver>(params);
		audioLevelObserver->handleWorkerNotifications();

      this->_rtpObservers[audioLevelObserver->id()] =  audioLevelObserver;
      audioLevelObserver->on("@close",[self = shared_from_this(), audioLevelObserver]( )
        {
            self->_rtpObservers.erase(audioLevelObserver->id());

        });


		// Emit observer event.
		this->_observer->safeEmit("newrtpobserver", audioLevelObserver);

		return audioLevelObserver;
	}

	/**
	 * Check whether the given RTP capabilities can consume the given Producer.
	 */
	bool canConsume(
		
			std::string producerId,
            RtpCapabilities &rtpCapabilities
	
	)
	{
        MS_lOGD(
                "canConsume() | Producer with id %s", producerId.c_str());
	    return true;
        auto &producer = this->_producers[producerId];
		if (!producer)
		{
			MS_lOGE(
				"canConsume() | Producer with id %s not found", producerId.c_str());

			return false;
		}

		try
		{
		    RtpParameters params = producer->consumableRtpParameters();
			return ortc::canConsume(params, rtpCapabilities);
		}
		catch (...)
		{
			MS_lOGE("canConsume() | unexpected error:");
			return false;
		}
	}
};


}
