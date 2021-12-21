#pragma once
#include <string>
#include <vector>
#include <json.hpp>
#include "Log.hpp"
#include "Transport.hpp"
#include "SctpParameters.hpp"
#include "SrtpParameters.hpp"
#include "Promise.hpp"
using json = nlohmann::json;
using namespace mediasoup;

namespace mediasoup
{
  
struct PlainTransportOptions
{
	/**
	 * Listening IP address.
	 */
   json listenIp;//: TransportListenIp | string;

	/**
	 * Use RTCP-mux (RTP and RTCP in the same port). Default true.
	 */
	bool  rtcpMux = true;

	/**
	 * Whether remote IP:port should be auto-detected based on first RTP/RTCP
	 * packet received. If enabled, connect() method must not be called unless
	 * SRTP is enabled. If so, it must be called with just remote SRTP parameters.
	 * Default false.
	 */
	bool comedia = false;

	/**
	 * Create a SCTP association. Default false.
	 */
	bool enableSctp = false;

	/**
	 * SCTP streams number.
	 */
    NumSctpStreams numSctpStreams;// = { .OS= 1024, .MIS= 1024 };//?: NumSctpStreams;

	/**
	 * Maximum allowed size for SCTP messages sent by DataProducers.
	 * Default 262144.
	 */
	int maxSctpMessageSize = 262144;

	/**
	 * Maximum SCTP send buffer used by DataConsumers.
	 * Default 262144.
	 */
	int sctpSendBufferSize = 262144;

	/**
	 * Enable SRTP. For this to work, connect() must be called
	 * with remote SRTP parameters. Default false.
	 */
	bool enableSrtp = false;

	/**
	 * The SRTP crypto suite to be used if enableSrtp is set. Default
	 * 'AES_CM_128_HMAC_SHA1_80'.
	 */
    //SrtpCryptoSuite srtpCryptoSuite;//?: SrtpCryptoSuite;
    std::string srtpCryptoSuite = "AES_CM_128_HMAC_SHA1_80";
	/**
	 * Custom application data.
	 */
	json _appData;
};
void to_json(json& j, const PlainTransportOptions& st);
void from_json(const json& j, PlainTransportOptions& st);
/**
 * DEPRECATED: Use PlainTransportOptions.
 */
//struct PlainRtpTransportOptions = PlainTransportOptions;

struct PlainTransportStat
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
	// PlainTransport specific.
	bool rtcpMux;
	bool comedia;
    TransportTuple tuple;//: TransportTuple;
    TransportTuple rtcpTuple;//?: ;
};

/**
 * DEPRECATED: Use PlainTransportStat.
 */
 //type PlainRtpTransportStat = PlainTransportStat;

//const logger = new Logger('PlainTransport');

 class PlainTransport : public Transport
{
public:
	// PlainTransport data.
    /*
	struct data:
	{
		bool rtcpMux;
		bool comedia;
		tuple: TransportTuple;
		rtcpTuple?: TransportTuple;
		sctpParameters?: SctpParameters;
		sctpState?: SctpState;
		srtpParameters?: SrtpParameters;
	};*/
    json _data;
	/**
	 * @private
	 * @emits tuple - (tuple: TransportTuple)
	 * @emits rtcptuple - (rtcpTuple: TransportTuple)
	 * @emits sctpstatechange - (sctpState: SctpState)
	 * @emits trace - (trace: TransportTraceEventData)
	 */
	PlainTransport(TransportParams &params) : Transport(params)
	{
		//super(params);

		MS_lOGD("PlainTransport()");

		json  &data = params.data;

		this->_data =
		{
			{"rtcpMux"        , data["rtcpMux"]},
			{"comedia"        , data["comedia"]},
			{"tuple"          , data["tuple"]},
			{"rtcpTuple"      , data["rtcpTuple"]},
			{"sctpParameters" , data["sctpParameters"]},
			{"sctpState"      , data["sctpState"]},
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
	 * Transport RTCP tuple.
	 */
	TransportTuple rtcpTuple()
	{
		return this->_data["rtcpTuple"];
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
	 * @emits tuple - (tuple: TransportTuple)
	 * @emits rtcptuple - (rtcpTuple: TransportTuple)
	 * @emits sctpstatechange - (sctpState: SctpState)
	 * @emits trace - (trace: TransportTraceEventData)
	 */
	EnhancedEventEmitter* observer()
	{
		return this->_observer;
	}
	/**
	 * Close the PlainTransport.
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
	 * PlainTransport stats.
	 *
	 * @override
	 */
	json getStats() // Promise<PlainTransportStat[]>
	{
		MS_lOGD("getStats()");

		return this->_channel->request("transport.getStats", this->_internal);
	}

	/**
	 * Provide the PlainTransport remote parameters.
	 *
	 * @override
	 */
    virtual void connect(
		json & params
    /*
			std::string &ip,
			int &port,
			int &rtcpPort,
			SrtpParameters &srtpParameters
		*/
	) // Promise<void>
	{
		MS_lOGD("connect()");

		const json reqData = {
            {"ip",params["ip"]},
            {"port",params["port"]},
            {"rtcpPort",params["rtcpPort"]},
            {"srtpParameters",params["srtpParameters"] }
      };

		json data =
			this->_channel->request("transport.connect", this->_internal, reqData);
        
		// Update data.
		if (data.contains("tuple"))
            this->_data["tuple"] = data["tuple"];

		if (data.contains("rtcpTuple"))
            this->_data["rtcpTuple"] = data["rtcpTuple"];

		this->_data["srtpParameters"] = data["srtpParameters"];
  }
  void processChannelNotifications(std::string event,const json & data) 
  {
    //	switch (event)
		//	{
				if(event == "tuple")
				{
					auto tuple = data["tuple"];// as TransportTuple;

					this->_data["tuple"] = tuple;

					this->safeEmit("tuple", tuple);

					// Emit observer event.
					this->_observer->safeEmit("tuple", tuple);

					//break;
				} else
				if(event == "rtcptuple")
				{
					auto rtcpTuple = data["rtcpTuple"];// as TransportTuple;

					this->_data["rtcpTuple"] = rtcpTuple;

					this->safeEmit("rtcptuple", rtcpTuple);

					// Emit observer event.
					this->_observer->safeEmit("rtcptuple", rtcpTuple);

					//break;
				} else
				if(event == "sctpstatechange")
				{
					auto sctpState = data["sctpState"];// as SctpState;

					this->_data["sctpState"] = sctpState;

					this->safeEmit("sctpstatechange", sctpState);

					// Emit observer event.
					this->_observer->safeEmit("sctpstatechange", sctpState);

					//break;
				}else
				if(event == "trace")
				{
					auto trace = data;// as TransportTraceEventData;

					this->safeEmit("trace", trace);

					// Emit observer event.
					this->_observer->safeEmit("trace", trace);

				//	break;
				}else
				{
					//MS_lOGE('ignoring unknown event "%s"', event.c_str());
				}
	//		}
  }
  void processPayloadChannelNotifications(std::string event,const json & data) 
  {
  }
	void handleWorkerNotifications()
	{
		this->_channel->on(this->_internal["transportId"].get<std::string>(),[self = Transport::downcasted_shared_from_this<PlainTransport>()]( std::string event,json data ) //this->_internal.transportId, (event, data?: any) =>
		{		 
			self->processChannelNotifications(event,data);
		});
	}
};

/**
 * DEPRECATED: Use PlainTransport.
 */
class PlainRtpTransport : public  PlainTransport
{
	PlainRtpTransport(TransportParams & params) : PlainTransport(params)
	{
		//super(params);
	}
};


}
