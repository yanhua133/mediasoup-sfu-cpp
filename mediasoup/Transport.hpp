#pragma once
#include "Log.hpp"
#include <string>
#include <vector>
#include <json.hpp>
#include "ChannelAgent.hpp"
#include "PayloadChannelAgent.hpp"
#include "Producer.hpp"
#include "Consumer.hpp"
#include "DataProducer.hpp"
#include "DataConsumer.hpp"
#include "RtpParameters.hpp"
#include "SctpParameters.hpp"
#include "EnhancedEventEmitter.hpp"
#include "Promise.hpp"
#include "uuid.hpp"
#include "ortc.hpp"
#include <memory>

using json = nlohmann::json;
using namespace uuid;
using namespace mediasoupclient;
//using namespace mediasoup;
//

namespace mediasoup
{
  
struct TransportListenIp
{
	/**
	 * Listening IPv4 or IPv6.
	 */
	std::string ip;

	/**
	 * Announced IPv4 or IPv6 (useful when running mediasoup behind NAT with
	 * IP).
	 */
	std::string announcedIp;
};

/**
 * Transport protocol.
 */
// type TransportProtocol = 'udp' | 'tcp';

struct TransportTuple
{
	std::string localIp;
	int localPort;
	std::string remoteIp;
	int remotePort;
	std::string protocol;//: TransportProtocol;
};
void to_json(json& j, const TransportTuple& st);
void from_json(const json& j, TransportTuple& st);
/**
 * Valid types for 'trace' event.
 */
//type TransportTraceEventType = 'probation' | 'bwe';

/**
 * 'trace' event data.
 */
struct TransportTraceEventData
{
	/**
	 * Trace type.
	 */
	std::string type;//: TransportTraceEventType;

	/**
	 * Event timestamp.
	 */
	int timestamp;

	/**
	 * Event direction.
	 */
	std::string direction;//: 'in' | 'out';

	/**
	 * Per type information.
	 */
	json info;//: any;
};
//typedef RtpCapabilities (*getRouterRtpCapabilities)();
//typedef Producer* (*getProducerById)(std::string );
//typedef DataProducer* (*getDataProducerById)(std::string);

struct TransportParams
{
		json internal;
		json data;
		std::shared_ptr<ChannelAgent> channel;
		std::shared_ptr<PayloadChannelAgent> payloadChannel;
		json appData;
    std::function<RtpCapabilities()> getRouterRtpCapabilities;
    std::function<std::shared_ptr<Producer>(std::string&)> getProducerById;
    std::function<std::shared_ptr<DataProducer>(std::string&)> getDataProducerById;
};

struct IceParameters
{
    std::string usernameFragment;
    std::string password;
    bool iceLite;
};

struct IceCandidate
{
    std::string foundation;
    int priority;
    std::string ip;
    std::string protocol;//: TransportProtocol;
    int port;
    std::string type;//: 'host';
    std::string tcpType;//: 'passive' | undefined;
};
/**
 * The hash function algorithm (as defined in the "Hash function Textual Names"
 * registry initially specified in RFC 4572 Section 8) and its corresponding
 * certificate fingerprint value (in lowercase hex string as expressed utilizing
 * the syntax of "fingerprint" in RFC 4572 Section 5).
 */
struct DtlsFingerprint
{
    std::string algorithm;
    std::string value;
};

struct DtlsParameters
{
    std::string role;//?: DtlsRole;
    std::vector<DtlsFingerprint> fingerprints;
};

void to_json(json& j, const IceParameters& st);
void from_json(const json& j, IceParameters& st);
void to_json(json& j, const IceCandidate& st);
void from_json(const json& j, IceCandidate& st);
void to_json(json& j, const DtlsFingerprint& st);
void from_json(const json& j, DtlsFingerprint& st);
void to_json(json& j, const DtlsParameters& st);
void from_json(const json& j, DtlsParameters& st);


 /* First a common base class
  * of course, one should always virtually inherit from it.
  */
class MultipleInheritableEnableSharedFromThis : public std::enable_shared_from_this<MultipleInheritableEnableSharedFromThis>
{
public:
	virtual ~MultipleInheritableEnableSharedFromThis()
	{}
};

template <class T>
class inheritable_enable_shared_from_this : virtual public MultipleInheritableEnableSharedFromThis
{
public:
	std::shared_ptr<T> shared_from_this() {
		return std::dynamic_pointer_cast<T>(MultipleInheritableEnableSharedFromThis::shared_from_this());
	}
	/* Utility method to easily downcast.
	 * Useful when a child doesn't inherit directly from enable_shared_from_this
	 * but wants to use the feature.
	 */
	template <class Down>
	std::shared_ptr<Down> downcasted_shared_from_this() {
		return std::dynamic_pointer_cast<Down>(MultipleInheritableEnableSharedFromThis::shared_from_this());
	}
};

//type SctpState = 'new' | 'connecting' | 'connected' | 'failed' | 'closed';

//const logger = new Logger('Transport");

class Transport  : public EnhancedEventEmitter, public inheritable_enable_shared_from_this<Transport>
{
	// Internal data.
public:
    /*
	struct _internal:
	{
		std::string routerId;
		std::string transportId;
	};*/
    json _internal;
	// Transport data. This is set by the subclass.
    /*
	struct data:
	{
		SctpParameters sctpParameters;
		SctpState sctpState;
	};*/
    json _data;
	// Channel instance.
	std::shared_ptr<ChannelAgent> _channel;

	// PayloadChannelAgent instance.
	std::shared_ptr<PayloadChannelAgent> _payloadChannel;

	// Close flag.
	bool _closed = false;

	// Custom app data.
    json _appData;

	// Method to retrieve Router RTP capabilities.
    std::function<RtpCapabilities()> _getRouterRtpCapabilities;//: () => RtpCapabilities;

	// Method to retrieve a Producer.
    std::function<std::shared_ptr<Producer>(std::string&)> _getProducerById;//: (producerId) => Producer;

	// Method to retrieve a DataProducer.
    std::function<std::shared_ptr<DataProducer>(std::string&)> _getDataProducerById;//: (dataProducerId) => DataProducer;

	// Producers map.
    std::unordered_map<std::string,std::shared_ptr<Producer> > _producers;

	// Consumers map.
    std::unordered_map<std::string,std::shared_ptr<Consumer> > _consumers;

	// DataProducers map.
    std::unordered_map<std::string,std::shared_ptr<DataProducer> > _dataProducers;

	// DataConsumers map.
    std::unordered_map<std::string,std::shared_ptr<DataConsumer> > _dataConsumers;

	// RTCP CNAME for Producers.
	std::string _cnameForProducers;

	// Next MID for Consumers. It's converted into string when used.
	int _nextMidForConsumers = 1;

	// Buffer with available SCTP stream ids.
	std::vector<int> _sctpStreamIds;

	// Next SCTP stream id.
	int _nextSctpStreamId = 1;

	// Observer instance.
    EnhancedEventEmitter * _observer = new EnhancedEventEmitter();

	/**
	 * @private
	 * @interface
	 * @emits routerclose
	 * @emits @close
	 * @emits @newproducer - (producer: Producer)
	 * @emits @producerclose - (producer: Producer)
	 * @emits @newdataproducer - (dataProducer: DataProducer)
	 * @emits @dataproducerclose - (dataProducer: DataProducer)
	 */
	Transport(
              TransportParams &params
               /*
    	json& internal,
			json& data,
			std::shared_ptr<ChannelAgent> channel,
			std::shared_ptr<PayloadChannelAgent> payloadChannel,
			json& appData,
            getRouterRtpCapabilities __getRouterRtpCapabilities, //RtpCapabilities
			getProducerById __getProducerById,  //Producer
			getDataProducerById __getDataProducerById  //DataProducer
                */
	)
	{
		//super();

		MS_lOGD("Transport()");

		this->_internal = params.internal;
		this->_data = params.data;
		this->_channel = params.channel;
		this->_payloadChannel = params.payloadChannel;
		this->_appData = params.appData;
		this->_getRouterRtpCapabilities = params.getRouterRtpCapabilities;
		this->_getProducerById = params.getProducerById;
		this->_getDataProducerById = params.getDataProducerById;
	}

	/**
	 * Transport id.
	 */
	std::string id()
	{
		return this->_internal["transportId"];
	}
	/**
	 * ICE parameters.
	 */
	IceParameters iceParameters()//: IceParameters
	{
		return this->_data["iceParameters"];
	}

	/**
	 * ICE candidates.
	 */
	std::vector<IceCandidate> iceCandidates()//: IceCandidate[]
	{
		return this->_data["iceCandidates"];
	}
  	/**
	 * DTLS parameters.
	 */
	DtlsParameters dtlsParameters()
	{
		return this->_data["dtlsParameters"];
	}

  	/**
	 * SCTP parameters.
	 */
	SctpParameters sctpParameters()
	{
		return this->_data["sctpParameters"];
	}
  
	/**
	 * Whether the Transport is closed.
	 */
	bool closed()
	{
		return this->_closed;
	}

	/**
	 * App custom data.
	 */
	json appData()
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
	 * Restart ICE.
	 */
	virtual json restartIce()//: Promise<IceParameters>
	{
		MS_lOGD("restartIce()");
		return json();
	}
	/**
	 * Observer.
	 *
	 * @emits close
	 * @emits newproducer - (producer: Producer)
	 * @emits newconsumer - (producer: Producer)
	 * @emits newdataproducer - (dataProducer: DataProducer)
	 * @emits newdataconsumer - (dataProducer: DataProducer)
	 */
	EnhancedEventEmitter* observer()
	{
		return this->_observer;
	}
	/**
	 * Close the Transport.
	 */
	void close()
	{
		if (this->_closed)
			return;

		MS_lOGD("close()");

		this->_closed = true;

		// Remove notification subscriptions.
		this->_channel->removeAllListeners(this->_internal["transportId"]);
		this->_payloadChannel->removeAllListeners(this->_internal["transportId"]);

		this->_channel->request("transport.close", this->_internal);

                                                  
		// Close every Producer.
        for (const auto& kv : this->_producers)
        {
            auto producerId        = kv.first;
            auto producer   = kv.second;

			producer->transportClosed();

			// Must tell the Router.
			this->emit("@producerclose", producer);
		}
		this->_producers.clear();

		// Close every Consumer.
        for (const auto& kv : this->_consumers)
        {
            auto consumerId        = kv.first;
            auto consumer   = kv.second;
		
			consumer->transportClosed();
		}
		this->_consumers.clear();

		// Close every DataProducer.
        for (const auto& kv : this->_dataProducers)
        {
            auto dataProducerId        = kv.first;
            auto dataProducer   = kv.second;
			dataProducer->transportClosed();

			// Must tell the Router.
			this->emit("@dataproducerclose", dataProducer);
		}
		this->_dataProducers.clear();

		// Close every DataConsumer.
        for (const auto& kv : this->_dataConsumers)
        {
            auto dataConsumerId        = kv.first;
            auto dataConsumer   = kv.second;
		
			dataConsumer->transportClosed();
		}
		this->_dataConsumers.clear();

		this->emit("@close");

		// Emit observer event.
		this->_observer->safeEmit("close");
	}

	/**
	 * Router was closed.
	 *
	 * @private
	 * @virtual
	 */
	void routerClosed()
	{
		if (this->_closed)
			return;

		MS_lOGD("routerClosed()");

		this->_closed = true;

		// Remove notification subscriptions.
		this->_channel->removeAllListeners(this->_internal["transportId"]);
		this->_payloadChannel->removeAllListeners(this->_internal["transportId"]);

		// Close every Producer.
        for (const auto& kv : this->_producers)
        {
            auto producerId        = kv.first;
            auto producer   = kv.second;

			producer->transportClosed();

			// NOTE: No need to tell the Router since it already knows (it has
			// been closed in fact).
		}
		this->_producers.clear();

		// Close every Consumer.
        for (const auto& kv : this->_consumers)
        {
            auto consumerId        = kv.first;
            auto consumer   = kv.second;
        
			consumer->transportClosed();
		}
		this->_consumers.clear();

		// Close every DataProducer.
        for (const auto& kv : this->_dataProducers)
        {
            auto dataProducerId        = kv.first;
            auto dataProducer   = kv.second;
			dataProducer->transportClosed();

			// NOTE: No need to tell the Router since it already knows (it has
			// been closed in fact).
		}
		this->_dataProducers.clear();

		// Close every DataConsumer.
        for (const auto& kv : this->_dataConsumers)
        {
            auto dataConsumerId        = kv.first;
            auto dataConsumer   = kv.second;
			dataConsumer->transportClosed();
		}
		this->_dataConsumers.clear();

		this->safeEmit("routerclose");

		// Emit observer event.
		this->_observer->safeEmit("close");
	}

	/**
	 * Dump Transport.
	 */
	json dump() //Promise<any>
	{
		MS_lOGD("dump()");

		return this->_channel->request("transport.dump", this->_internal);
	}

	/**
	 * Transport stats.
	 *
	 * @abstract
	 */
	json getStats()// Promise<any[]>
	{
		// Should not happen.
		MS_THROW_lOG("method not implemented in the subclass");
	}

	/**
	 * Provide the Transport remote parameters.
	 *
	 * @abstract
	 */
	// eslint-disable-next-line @typescript-eslint/no-unused-vars
    virtual void connect(json &params) // Promise<void>
	{
		// Should not happen.
		MS_THROW_lOG("method not implemented in the subclass");
	}

	/**
	 * Set maximum incoming bitrate for receiving media.
	 */
	void setMaxIncomingBitrate(int bitrate)// Promise<void>
	{
		MS_lOGD("setMaxIncomingBitrate() [bitrate:%d]", bitrate);

		const json reqData = {
            { "bitrate",bitrate }
        };

		this->_channel->request(
			"transport.setMaxIncomingBitrate", this->_internal, reqData);
	}

	/**
	 * Create a Producer.
	 */
    std::shared_ptr<Producer>  produce(
	  ProducerOptions & options
	) // Promise<Producer>
	{
		MS_lOGD("produce()");
    	std::string &id = options.id;
      std::string &kind = options.kind;
        RtpParameters &rtpParameters = options.rtpParameters;
			bool &paused = options.paused;
			int &keyFrameRequestDelay = options.keyFrameRequestDelay;
        
			json &appData= options.appData;

		if (this->_producers.find(id) != this->_producers.end())
        {
            MS_THROW_lOG("a Producer with same id=%s already exists",id.c_str());
        }
		else if (kind != "audio" && kind != "video")
        {
            MS_THROW_lOG("invalid kind %s",kind.c_str());
        }
		//else if (appData && typeof appData !== 'object')
		//	MS_THROW_lOG("if given, appData must be an object");

		// This may throw.
        json jrtpParameters=rtpParameters;
        ortc::validateRtpParameters(jrtpParameters);

		// If missing or empty encodings, add one.
        /*
		if (
			!rtpParameters.encodings ||
			!Array.isArray(rtpParameters.encodings) ||
			rtpParameters.encodings.length === 0
		)
		{
			rtpParameters.encodings = [ {} ];
		}
*/
		// Don't do this in PipeTransports since there we must keep CNAME value in
		// each Producer.
		if (strcmp(typeid(*this).name() ,"PipeTransport") != 0)
		{
			// If CNAME is given and we don't have yet a CNAME for Producers in this
			// Transport, take it.
			if (this->_cnameForProducers.empty() && !rtpParameters.rtcp.cname.empty())
			{
				this->_cnameForProducers = rtpParameters.rtcp.cname;
			}
			// Otherwise if we don't have yet a CNAME for Producers and the RTP parameters
			// do not include CNAME, create a random one.
			else if (!this->_cnameForProducers.empty())
			{
                this->_cnameForProducers = uuidv4_prefix8();//.substr(0, 8);
			}

			// Override Producer's CNAME.
			//rtpParameters.rtcp = rtpParameters.rtcp || {};
			rtpParameters.rtcp.cname = this->_cnameForProducers;
		}

        auto  routerRtpCapabilities = this->_getRouterRtpCapabilities();

		// This may throw.
      auto  rtpMapping = ortc::getProducerRtpParametersMapping(
			rtpParameters, routerRtpCapabilities);

		// This may throw.
		MS_lOGD("produce rtpMapping=%s",rtpMapping.dump(4).c_str());

        jrtpParameters=rtpParameters;
        MS_lOGD("produce jrtpParameters=%s",jrtpParameters.dump(4).c_str());
        RtpParameters consumableRtpParameters = ortc::getConsumableRtpParameters(
			kind, rtpParameters, routerRtpCapabilities, rtpMapping);
        json jconsumableRtpParameters = consumableRtpParameters;
        MS_lOGD("getConsumableRtpParameters jconsumableRtpParameters=%s",jconsumableRtpParameters.dump(4).c_str());
		//const internal = { ...this->_internal, producerId: id || uuidv4() };
        json internal = this->_internal;
		internal["producerId"] = id.empty() ? uuidv4() : id;
        
		const json reqData = {
            {"kind",kind},
            {"rtpParameters",rtpParameters},
            {"rtpMapping",rtpMapping},
            {"keyFrameRequestDelay",keyFrameRequestDelay},
            {"paused",paused}
        };
        MS_lOGD("produce transport.produce=%s",reqData.dump(4).c_str());
		//const status =
		json status =	this->_channel->request("transport.produce", internal, reqData);

    
        json data = {
            {"kind",kind},
            {"rtpParameters",rtpParameters},
            {"type" , status["type"]},
            {"consumableRtpParameters",consumableRtpParameters}
        };
        MS_lOGD("produce std::make_shared<Producer>=%s",data.dump(4).c_str());
        std::shared_ptr<Producer> producer = std::make_shared<Producer>(
			//{
				internal,
				data,
				this->_channel,
				this->_payloadChannel,
				appData,
				paused
			//}
        );
		producer->handleWorkerNotifications();

		this->_producers[producer->id()] =  producer;

		producer->on("@close",[self = shared_from_this(), producer]()
		{
			self->_producers.erase(producer->id());
			self->emit("@producerclose", producer);
		});

		this->emit("@newproducer", producer);

		// Emit observer event.
		this->_observer->safeEmit("newproducer",producer);

		return producer;
	}

	/**
	 * Create a Consumer.
	 *
	 * @virtual
	 */
    std::shared_ptr<Consumer>  consume(
	    ConsumerOptions &options
	) // Promise<Consumer>
	{
		MS_lOGD("consume()");
	    std::string &producerId = options.producerId;
        RtpCapabilities &rtpCapabilities = options.rtpCapabilities;
        bool paused = options.paused;
        ConsumerLayers preferredLayers = options.preferredLayers;
        bool pipe = options.pipe;
        json& appData = options.appData;
		//if (!producerId || typeof producerId !== 'string')
		//	MS_THROW_lOG("missing producerId");
		//else if (appData && typeof appData !== 'object')
		//	MS_THROW_lOG("if given, appData must be an object");

        if(producerId.empty()) {
            MS_THROW_lOG("missing producerId");
        }
		// This may throw.
		json jRtpCapabilities =  rtpCapabilities;
		ortc::validateRtpCapabilities(jRtpCapabilities);

		auto  producer = this->_getProducerById(producerId);

        if (!producer) {
            MS_THROW_lOG("Producer with id %s not found",producerId.c_str());
        }
			

		// This may throw.
		auto rtpParameters = ortc::getConsumerRtpParameters(
			producer->consumableRtpParameters(), rtpCapabilities, pipe);
        json jrtpParameters = rtpParameters;
        json jconsumableRtpParameters = producer->consumableRtpParameters();
        MS_lOGD("producer->consumableRtpParameters() consumableRtpParameters=%s",jconsumableRtpParameters.dump(4).c_str());
        MS_lOGD("ortc::getConsumerRtpParameters rtpParameters=%s",jrtpParameters.dump(4).c_str());
		// Set MID.

		if (!pipe)
		{
			rtpParameters.mid = std::to_string(this->_nextMidForConsumers++);

			// We use up to 8 bytes for MID (string).
			if (this->_nextMidForConsumers == 100000000)
			{
				MS_lOGE(
					"consume() | reaching max MID value _nextMidForConsumers=%d",this->_nextMidForConsumers);

				this->_nextMidForConsumers = 0;
			}
		}
        //const internal = { ...this->_internal, consumerId: uuidv4(), producerId };
        json internal = this->_internal;
		internal["consumerId"] = uuidv4();
        internal["producerId"] = producerId;
        
       
		const json reqData = {
            {"kind"                   , producer->kind()},
            {"rtpParameters",rtpParameters},
            {"type"                   , pipe ? "pipe" : producer->type()},
            {"consumableRtpEncodings" , producer->consumableRtpParameters().encodings},
            {"paused",paused},
            {"preferredLayers",preferredLayers}
        };

		json status =
			this->_channel->request("transport.consume", internal, reqData);

		json data = {
            {"kind" , producer->kind()},
            {"rtpParameters" , rtpParameters},
            {"type" , pipe ? "pipe" : producer->type()}
        };
        bool paused1 = status["paused"];
        bool producerPaused1=status["producerPaused"];
        ConsumerScore score1=status["score"];
        ConsumerLayers preferredLayers1=status["preferredLayers"];
        std::shared_ptr<Consumer> consumer = std::make_shared<Consumer>(
			//{
				internal,
				data,
				this->_channel,
				this->_payloadChannel,
				appData,
                paused1,//status["paused"],
                producerPaused1,//status["producerPaused"],
                score1,//status["score"],
                preferredLayers1//status["preferredLayers"]
			//}
        );
		consumer->handleWorkerNotifications();

		this->_consumers[consumer->id()] =  consumer;
		//consumer.on('@close', () => this->_consumers.delete(consumer.id));
        consumer->on("@close",[self = shared_from_this(), consumer](  )
        {
            self->_consumers.erase(consumer->id());

        });
		//consumer.on('@producerclose', () => this->_consumers.delete(consumer.id));
        consumer->on("@producerclose",[self = shared_from_this(), consumer](  )
        {
            self->_consumers.erase(consumer->id());
        });

		// Emit observer event.
		this->_observer->safeEmit("newconsumer", consumer);

		return consumer;
	}

	/**
	 * Create a DataProducer.
	 */
    std::shared_ptr<DataProducer>  produceData(
      DataProducerOptions &options
	) // Promise<DataProducer>
	{
    	std::string &id = options.id;
      SctpStreamParameters &sctpStreamParameters= options.sctpStreamParameters;
      std::string &label= options.label;
      std::string &protocol= options.protocol;
			json &appData= options._appData;
		MS_lOGD("produceData()");
        if (this->_dataProducers.find(id) != this->_dataProducers.end())
        {
            MS_THROW_lOG("a DataProducer with same id=%s already exists",id.c_str());
        }
		//if (id && this->_dataProducers.has(id))
		//	MS_THROW_lOG(`a DataProducer with same id "${id}" already exists`);
		//else if (appData && typeof appData !== 'object')
		//	MS_THROW_lOG("if given, appData must be an object");

		//let type: DataProducerType;
        std::string type;
		// If this is not a DirectTransport, sctpStreamParameters are required.
		//if (this->constructor.name !== 'DirectTransport')
        if (strcmp(typeid(*this).name() ,"DirectTransport") != 0)
		{
			type = "sctp";

			// This may throw.
			//ortc::validateSctpStreamParameters(sctpStreamParameters!);
		}
		// If this is a DirectTransport, sctpStreamParameters must not be given.
		else
		{
			type = "direct";

			//if (sctpStreamParameters)
			//{
			//	MS_LOGW(
			//		"produceData() | sctpStreamParameters are ignored when producing data on a DirectTransport");
			//}
		}

		//const internal = { ...this->_internal, dataProducerId: id || uuidv4() };
        json internal = this->_internal;
        
        internal["dataProducerId"] = id.empty() ? uuidv4() : id;
        
		const json reqData = {
            {"type",type},
            {"sctpStreamParameters",sctpStreamParameters},
            {"label",label},
            {"protocol",protocol}
        };

		json data =
			this->_channel->request("transport.produceData", internal, reqData);
        json data1;
        std::shared_ptr<DataProducer> dataProducer = std::make_shared<DataProducer>(
			//{
				internal,
                data1,
				this->_channel,
				this->_payloadChannel,
				appData
		//	}
        );

		this->_dataProducers[dataProducer->id()] = dataProducer;
		dataProducer->on("@close",[](const event::event_t<json>& e )
		{
			//this->_dataProducers.delete(dataProducer.id);
			//this->emit("@dataproducerclose", dataProducer);
		});

		this->emit("@newdataproducer", dataProducer);

		// Emit observer event.
		this->_observer->safeEmit("newdataproducer", dataProducer);

		return dataProducer;
	}

	/**
	 * Create a DataConsumer.
	 */
    std::shared_ptr<DataConsumer>   consumeData(
	  DataConsumerOptions & options
	) // Promise<DataConsumer>
	{
    	std::string dataProducerId = options.dataProducerId;
			bool ordered = options.ordered;
			int maxPacketLifeTime = options.maxPacketLifeTime;
			int maxRetransmits = options.maxRetransmits;
			json & appData = options._appData;
    
		MS_lOGD("consumeData()");
        if(dataProducerId.empty()) {
            MS_THROW_lOG("missing producerId");
        }
		//if (!dataProducerId || typeof dataProducerId !== 'string')
		//	MS_THROW_lOG("missing dataProducerId");
		//else if (appData && typeof appData !== 'object')
		//	MS_THROW_lOG("if given, appData must be an object");

		auto dataProducer = this->_getDataProducerById(dataProducerId);

        if (!dataProducer) {
            MS_THROW_lOG("dataProducer with id %s not found",dataProducerId.c_str());
        }
            

        std::string type;//: DataConsumerType;
        SctpStreamParameters sctpStreamParameters;//: SctpStreamParameters | undefined;
		int sctpStreamId=0;

		// If this is not a DirectTransport, use sctpStreamParameters from the
		// DataProducer (if type 'sctp') unless they are given in method parameters.
		//if (this->constructor.name !== 'DirectTransport')
        if (strcmp(typeid(*this).name() ,"DirectTransport") != 0)
		{
			type = "sctp";
            
            sctpStreamParameters = dataProducer->sctpStreamParameters();
				//utils.clone(dataProducer.sctpStreamParameters) as SctpStreamParameters;

			// Override if given.
			//if (ordered !== undefined)
            sctpStreamParameters.ordered = ordered;

			//if (maxPacketLifeTime !== undefined)
            sctpStreamParameters.maxPacketLifeTime = maxPacketLifeTime;

			//if (maxRetransmits !== undefined)
            sctpStreamParameters.maxRetransmits = maxRetransmits;

			// This may throw.
			sctpStreamId = this->_getNextSctpStreamId();

			this->_sctpStreamIds[sctpStreamId] = 1;
			sctpStreamParameters.streamId = sctpStreamId;
             
		}
		// If this is a DirectTransport, sctpStreamParameters must not be used.
		else
		{
			type = "direct";
/*
			if (
				ordered !== undefined ||
				maxPacketLifeTime !== undefined ||
				maxRetransmits !== undefined
			)
			{
				MS_LOGW(
					"consumeData() | ordered, maxPacketLifeTime and maxRetransmits are ignored when consuming data on a DirectTransport");
			}
 */
		}

		//const { label, protocol } = dataProducer;
        auto label = dataProducer->label();
        auto protocol = dataProducer->protocol();
		//const internal = { ...this->_internal, dataConsumerId: uuidv4(), dataProducerId };
        json internal = this->_internal;
		internal["dataConsumerId"] = uuidv4();
        internal["dataProducerId"] = dataProducerId;
		const json reqData = {
            {"type",type},
            {"sctpStreamParameters",sctpStreamParameters},
            {"label",label},
            {"protocol",protocol}
        };

		json data =
			this->_channel->request("transport.consumeData", internal, reqData);
        json data1;
        std::shared_ptr<DataConsumer> dataConsumer = std::make_shared<DataConsumer>(
			//{
				internal,
				data1,
				this->_channel,
				this->_payloadChannel,
				appData
			//}
        );
		dataConsumer->handleWorkerNotifications();

		this->_dataConsumers[dataConsumer->id()] = dataConsumer;
		dataConsumer->on("@close",[self = shared_from_this(), dataConsumer, sctpStreamId](  )
		{
			self->_dataConsumers.erase(dataConsumer->id());

			if (self->_sctpStreamIds.size() != 0)
				self->_sctpStreamIds[sctpStreamId] = 0;
		});
		dataConsumer->on("@dataproducerclose",[self = shared_from_this(), dataConsumer, sctpStreamId](  )
		{
            self->_dataConsumers.erase(dataConsumer->id());

            if (self->_sctpStreamIds.size() != 0)
				self->_sctpStreamIds[sctpStreamId] = 0;
		});

		// Emit observer event.
		this->_observer->safeEmit("newdataconsumer", dataConsumer);

		return dataConsumer;
	}

	/**
	 * Enable 'trace' event.
	 */
	void enableTraceEvent(std::vector<std::string> types) //Promise<void>
	{
		MS_lOGD("pause()");

		const json reqData = {
            { "types",types }
      };

		this->_channel->request(
			"transport.enableTraceEvent", this->_internal, reqData);
	}

	int _getNextSctpStreamId()
	{
        
		if (
			!this->_data["sctpParameters"] ||
            //strcmp(typeid(*this).name() ,"DirectTransport")
            this->_data["sctpParameters"]["MIS"] == 0
		)
		{
			MS_THROW_lOG("missing data.sctpParameters.MIS");
		}

		const int numStreams = this->_data["sctpParameters"]["MIS"];

        if (this->_sctpStreamIds.size() == 0) {
            //this->_sctpStreamIds = Buffer.alloc(numStreams, 0);
            for(int i=0;i<numStreams;i++) {
                this->_sctpStreamIds.push_back(0);
            }
        }
			

		int sctpStreamId;

		for (int idx = 0; idx < this->_sctpStreamIds.size(); ++idx)
		{
			sctpStreamId = (this->_nextSctpStreamId + idx) % this->_sctpStreamIds.size();

			if (!this->_sctpStreamIds[sctpStreamId])
			{
				this->_nextSctpStreamId = sctpStreamId + 1;

				return sctpStreamId;
			}
		}

		MS_THROW_lOG("no sctpStreamId available");
	}
 };


}
