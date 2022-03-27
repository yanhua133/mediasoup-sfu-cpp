#pragma once
#include <string>
#include <vector>
#include <json.hpp>
#include "Log.hpp"
#include "Transport.hpp"
#include "Consumer.hpp"
#include "RtpParameters.hpp"
#include "SctpParameters.hpp"
#include "SrtpParameters.hpp"
#include "Promise.hpp"
using json = nlohmann::json;
using namespace mediasoup;


namespace mediasoup
{
  
struct PipeTransportOptions
{
	/**
	 * Listening IP address.
	 */
    std::string listenIp;//: TransportListenIp | string;
	/**
	 * Create a SCTP association. Default false.
	 */
	bool enableSctp = false;

	/**
	 * SCTP streams number.
	 */
    NumSctpStreams numSctpStreams;// = { .OS =  1024, .MIS =  1024 };//?: NumSctpStreams;

	/**
	 * Maximum allowed size for SCTP messages sent by DataProducers.
	 * Default 268435456.
	 */
	int maxSctpMessageSize = 268435456;

	/**
	 * Maximum SCTP send buffer used by DataConsumers.
	 * Default 268435456.
	 */
	int sctpSendBufferSize = 268435456;

	/**
	 * Enable RTX and NACK for RTP retransmission. Useful if both Routers are
	 * located in different hosts and there is packet lost in the link. For this
	 * to work, both PipeTransports must enable this setting. Default false.
	 */
	bool enableRtx = false;

	/**
	 * Enable SRTP. Useful to protect the RTP and RTCP traffic if both Routers
	 * are located in different hosts. For this to work, connect() must be called
	 * with remote SRTP parameters. Default false.
	 */
	bool enableSrtp = false;

	/**
	 * Custom application data.
	 */
	json _appData;
};
void to_json(json& j, const PipeTransportOptions& st);
void from_json(const json& j, PipeTransportOptions& st);

struct PipeTransportStat
{
	// Common to all Transports.
	std::string type;
	std::string transportId;
	int timestamp;
    std::string sctpState;//?: SctpState;
	int bytesReceived;
	int recvBitrate;
	int bytesSent;
	int sendBitrate;
	int rtpBytesReceived;
	int rtpRecvBitrate;
	int rtpBytesSent;
	int rtpSendBitrate;
	int rtxBytesReceived;
	int rtxRecvBitrate;
	int rtxBytesSent;
	int rtxSendBitrate;
	int probationBytesSent;
	int probationSendBitrate;
	int availableOutgoingBitrate;
	int availableIncomingBitrate;
	int maxIncomingBitrate;
	// PipeTransport specific.
    TransportTuple tuple;
};

//const logger = new Logger('PipeTransport');

 class PipeTransport : public Transport
{
public:
	// PipeTransport data.
    /*
	struct data:
	{
		tuple: TransportTuple;
		sctpParameters?: SctpParameters;
		sctpState?: SctpState;
		rtx: boolean;
		srtpParameters?: SrtpParameters;
	};*/
  json _data;
	/**
	 * @private
	 * @emits sctpstatechange - (sctpState: SctpState)
	 * @emits trace - (trace: TransportTraceEventData)
	 */
	PipeTransport(TransportParams &params) : Transport(params)
	{
		//super(params);

		MS_lOGD("PipeTransport()");

		json  &data = params.data;

		this->_data =
		{
			{"tuple"          , data["tuple"]},
			{"sctpParameters" , data["sctpParameters"]},
			{"sctpState"      , data["sctpState"]},
			{"rtx"            , data["rtx"]},
			{"srtpParameters" , data["srtpParameters"]}
		};

		//this->_handleWorkerNotifications();
	}

	/**
	 * Transport tuple.
	 */
	TransportTuple tuple()
	{
		return this->_data["tuple"];
	}

	/**
	 * SCTP parameters.
	 */
	SctpParameters sctpParameters()
	{
		return this->_data["sctpParameters"];
	}

	/**
	 * SCTP state.
	 */
	std::string sctpState()
	{
		return this->_data["sctpState"];
	}

	/**
	 * SRTP parameters.
	 */
	SrtpParameters srtpParameters()
	{
		return this->_data["srtpParameters"];
	}

	/**
	 * Observer.
	 *
	 * @override
	 * @emits close
	 * @emits newproducer - (producer: Producer)
	 * @emits newconsumer - (consumer: Consumer)
	 * @emits newdataproducer - (dataProducer: DataProducer)
	 * @emits newdataconsumer - (dataConsumer: DataConsumer)
	 * @emits sctpstatechange - (sctpState: SctpState)
	 * @emits trace - (trace: TransportTraceEventData)
	 */
	EnhancedEventEmitter* observer()
	{
		return this->_observer;
	}

	/**
	 * Close the PipeTransport.
	 *
	 * @override
	 */
	void close()
	{
		if (this->_closed)
			return;

		//if (this->_data.sctpState)
        this->_data["sctpState"] = "closed";

        Transport::close();
	}

	/**
	 * Router was closed.
	 *
	 * @private
	 * @override
	 */
	void routerClosed()
	{
		if (this->_closed)
			return;

		//if (this->_data.sctpState)
        this->_data["sctpState"] = "closed";

		Transport::routerClosed();
	}

	/**
	 * PipeTransport stats.
	 *
	 * @override
	 */
	json getStats()// Promise<PipeTransportStat[]>
	{
		MS_lOGD("getStats()");

		return this->_channel->request("transport.getStats", this->_internal);
	}

	/**
	 * Provide the PipeTransport remote parameters.
	 *
	 * @override
	 */
    virtual void connect(
		//{
      	json & params
        /*
			std::string ip,
			int port,
            SrtpParameters &srtpParameters*/
		//}:
		//{
		//	ip;
		//	port;
		//	srtpParameters?: SrtpParameters;
		//}
	) // Promise<void>
	{
		MS_lOGD("connect()");

		const json reqData = {
            { "ip",params["ip"] },
            {"port",params["port"] },
            {"srtpParameters",params["srtpParameters"] }
      };

		json data =
			this->_channel->request("transport.connect", this->_internal, reqData);

		// Update data.
		this->_data["tuple"] = data["tuple"];
	}

	/**
	 * Create a pipe Consumer.
	 *
	 * @override
	 */
    std::shared_ptr<Consumer> consume(std::string producerId, json & appData )//}: ConsumerOptions)// Promise<Consumer>
	{
		MS_lOGD("consume()");

        if(producerId.empty()) {
            MS_THROW_lOG("missing producerId");
        }
		//if (!producerId || typeof producerId !== 'string')
		//	MS_THROW_lOG("missing producerId");
		//else if (appData && typeof appData !== 'object')
		//	MS_THROW_lOG("if given, appData must be an object");

		auto producer = this->_getProducerById(producerId);

        if (!producer) {
            MS_THROW_lOG("Producer with id %s not found",producerId.c_str());
        }
			

		// This may throw.
		//const rtpParameters = ortc.getPipeConsumerRtpParameters(
		//	producer.consumableRtpParameters, this->_data.rtx);
        RtpParameters rtpParameters;
		//const internal = { ...this->_internal, consumerId: uuidv4(), producerId };
        json internal = this->_internal;
        internal["consumerId"] = uuidv4();
        internal["producerId"] = producerId;
        
		const json reqData = {
            {"kind"                   , producer->kind() },
            {"rtpParameters"          , rtpParameters },
            {"type"                   , "pipe" },
            {"consumableRtpEncodings" , producer->consumableRtpParameters().encodings }
        };

		json status =
			this->_channel->request("transport.consume", internal, reqData);

        json data = {
            {"kind", producer->kind() },
            {"rtpParameters", rtpParameters },
            {"type", "pipe" }
        };
        ConsumerScore score;
        ConsumerLayers preferredLayers;
        std::shared_ptr<Consumer> consumer = std::make_shared<Consumer>(
			//{
				internal,
				data,
				this->_channel,
				this->_payloadChannel,
				appData,
                true,//paused         : status.paused,
				true,//producerPaused : status.producerPaused
                score,
                preferredLayers
			//}
        );
		consumer->handleWorkerNotifications();
		
		this->_consumers[consumer->id()] = consumer;
	//consumer.on('@close', () => this->_consumers.delete(consumer.id));
        consumer->on("@close",[self = Transport::downcasted_shared_from_this<PipeTransport>(), id = consumer->id()]( )
        {
            self->_consumers.erase(id);

        });
		//consumer.on('@producerclose', () => this->_consumers.delete(consumer.id));
        consumer->on("@producerclose",[self = Transport::downcasted_shared_from_this<PipeTransport>(), id = consumer->id()](  )
        {
            self->_consumers.erase(id);
        });
		// Emit observer event.
		this->_observer->safeEmit("newconsumer", consumer);

		return consumer;
  }
  void processChannelNotifications(std::string event,const json & data) 
  {
    //switch (event)
			//{
				if(event == "sctpstatechange")
				{
					auto sctpState = data["sctpState"];// as SctpState;

                    this->_data["sctpState"] = sctpState;

					this->safeEmit("sctpstatechange", sctpState);

					// Emit observer event.
					this->_observer->safeEmit("sctpstatechange", sctpState);

				//	break;
				} else 
				if(event == "trace")
				{
					auto trace = data ;//as TransportTraceEventData;

					this->safeEmit("trace", trace);

					// Emit observer event.
					this->_observer->safeEmit("trace", trace);

				//	break;
				} else 
				{
					//MS_lOGE('ignoring unknown event "%s"', event.c_str());
				}
			//}
  }
  void processPayloadChannelNotifications(std::string event,const json & data) 
  {
  }
	void handleWorkerNotifications()
	{
		this->_channel->on(this->_internal["transportId"].get<std::string>(),[self = Transport::downcasted_shared_from_this<PipeTransport>()]( std::string event,json data ) //this->_internal.transportId, (event, data?: any) =>
		{		
			self->processChannelNotifications(event,data);
		});
	}
};


}
