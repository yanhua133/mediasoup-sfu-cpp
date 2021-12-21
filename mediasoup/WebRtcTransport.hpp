#pragma once
#include "Log.hpp"
#include "EnhancedEventEmitter.hpp"
#include "SctpParameters.hpp"
#include "Transport.hpp"
#include "json.hpp"

#include "Promise.hpp"
using json = nlohmann::json;
using namespace mediasoup;


namespace mediasoup
{
  
struct WebRtcTransportOptions
{
	/**
	 * Listening IP address or addresses in order of preference (first one is the
	 * preferred one).
	 */
	json listenIps;//: (TransportListenIp | string)[];
	/**
	 * Listen in UDP. Default true.
	 */
	bool enableUdp = true;

	/**
	 * Listen in TCP. Default false.
	 */
	bool enableTcp = false;

	/**
	 * Prefer UDP. Default false.
	 */
	bool preferUdp = false;

	/**
	 * Prefer TCP. Default false.
	 */
	bool preferTcp = false;

	/**
	 * Initial available outgoing bitrate (in bps). Default 600000.
	 */
	int initialAvailableOutgoingBitrate = 600000;

	/**
	 * Create a SCTP association. Default false.
	 */
	bool enableSctp = false;

	/**
	 * SCTP streams number.
	 */
    NumSctpStreams numSctpStreams;//={ .OS= 1024, .MIS= 1024 };//?: NumSctpStreams;

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
	 * Custom application data.
	 */
	json appData;
};


//struct IceState = 'new' | 'connected' | 'completed' | 'disconnected' | 'closed';

//struct DtlsRole = 'auto' | 'client' | 'server';

//struct DtlsState = 'new' | 'connecting' | 'connected' | 'failed' | 'closed';

struct WebRtcTransportStat
{
	// Common to all Transports.
	std::string type;
	std::string transportId;
	int timestamp;
	std::string sctpState;
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
	// WebRtcTransport specific.
	std::string iceRole;
    std::string iiceState;//: IceState;
    TransportTuple iiceSelectedTuple;//?: TransportTuple;
    std::string idtlsState;//: DtlsState;
};
void to_json(json& j, const WebRtcTransportOptions& st);
void from_json(const json& j, WebRtcTransportOptions& st);
//const logger = new Logger('WebRtcTransport');

class WebRtcTransport : public Transport
{
	// WebRtcTransport data.
public:
    /*
	_data:
	{
		iceRole: 'controlled';
		iceParameters: IceParameters;
		iceCandidates: IceCandidate[];
		iceState: IceState;
		iceSelectedTuple?: TransportTuple;
		dtlsParameters: DtlsParameters;
		dtlsState: DtlsState;
		std::string dtlsRemoteCert?;
		sctpParameters?: SctpParameters;
		sctpState?: SctpState;
	};
*/
    json _data;
	/**
	 * @private
	 * @emits icestatechange - (iceState: IceState)
	 * @emits iceselectedtuplechange - (iceSelectedTuple: TransportTuple)
	 * @emits dtlsstatechange - (dtlsState: DtlsState)
	 * @emits sctpstatechange - (sctpState: SctpState)
	 * @emits trace - (trace: TransportTraceEventData)
	 */
    WebRtcTransport(TransportParams &params): Transport(params)
	{
        //Transport::Transport(params);

		MS_lOGD("WebRtcTransport()");

		json  &data = params.data;

		this->_data ={
            {"iceRole"          , data["iceRole"]},
            {"iceParameters"          , data["iceParameters"]},
            {"iceCandidates"          , data["iceCandidates"]},
            {"iceState"          , data["iceState"]},
            {"iceSelectedTuple"          , data["iceSelectedTuple"]},
            {"dtlsParameters"          , data["dtlsParameters"]},
            {"dtlsState"          , data["dtlsState"]},
            {"dtlsRemoteCert"          , data["dtlsRemoteCert"]},
            {"sctpParameters"          , data["sctpParameters"]},
            {"sctpState"          , data["sctpState"]}
        };

		//this->_handleWorkerNotifications();
	}

	/**
	 * ICE role.
	 */
	std::string iceRole()//: 'controlled'
	{
		return this->_data["iceRole"];
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
	 * ICE state.
	 */
	std::string iceState()
	{
		return this->_data["iceState"];
	}

	/**
	 * ICE selected tuple.
	 */
	TransportTuple iceSelectedTuple()
	{
		return this->_data["iceSelectedTuple"];
	}

	/**
	 * DTLS parameters.
	 */
	DtlsParameters dtlsParameters()
	{
		return this->_data["dtlsParameters"];
	}

	/**
	 * DTLS state.
	 */
	std::string dtlsState()
	{
		return this->_data["dtlsState"];
	}

	/**
	 * Remote certificate in PEM format.
	 */
	std::string dtlsRemoteCert()
	{
		return this->_data["dtlsRemoteCert"];
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
	 * Observer.
	 *
	 * @override
	 * @emits close
	 * @emits newproducer - (producer: Producer)
	 * @emits newconsumer - (consumer: Consumer)
	 * @emits newdataproducer - (dataProducer: DataProducer)
	 * @emits newdataconsumer - (dataConsumer: DataConsumer)
	 * @emits icestatechange - (iceState: IceState)
	 * @emits iceselectedtuplechange - (iceSelectedTuple: TransportTuple)
	 * @emits dtlsstatechange - (dtlsState: DtlsState)
	 * @emits sctpstatechange - (sctpState: SctpState)
	 * @emits trace - (trace: TransportTraceEventData)
	 */
	EnhancedEventEmitter *observer()
	{
		return this->_observer;
	}

	/**
	 * Close the WebRtcTransport.
	 *
	 * @override
	 */
	void close()
	{
		if (this->_closed)
			return;

	this->_data["iceState"] = "closed";
		this->_data["iceSelectedTuple"] = "";
		this->_data["dtlsState"] = "closed";

		if (this->_data["sctpState"])
			this->_data["sctpState"] = "closed";

		//super.close();
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

		this->_data["iceState"] = "closed";
		this->_data["iceSelectedTuple"] = "";
		this->_data["dtlsState"] = "closed";

		if (this->_data["sctpState"])
			this->_data["sctpState"] = "closed";

		//super.routerClosed();
    Transport::routerClosed();
	}

	/**
	 * WebRtcTransport stats.
	 *
	 * @override
	 */
	virtual 	json getStats()//: Promise<WebRtcTransportStat[]>
	{
		MS_lOGD("getStats()");

		return this->_channel->request("transport.getStats", this->_internal);
	}

	/**
	 * Provide the WebRtcTransport remote parameters.
	 *
	 * @override
	 */
    virtual void connect(
    json & params
   // DtlsParameters &dtlsParameters
    
    )// }: { dtlsParameters: DtlsParameters }): Promise<void>
	{
		MS_lOGD("connect()");

        const json reqData = { 
          {"dtlsParameters",params["dtlsParameters"]} 
        };

		json data =
			this->_channel->request("transport.connect", this->_internal, reqData);

		// Update data.
		this->_data["dtlsParameters"]["role"] = data["dtlsLocalRole"];
	}

	/**
	 * Restart ICE.
	 */
    virtual json restartIce()//: Promise<IceParameters>
	{
		MS_lOGD("restartIce() this->_internal=%s",this->_internal.dump(4).c_str());

		json data =
			this->_channel->request("transport.restartIce", this->_internal);

		this->_data["iceParameters"] = data["iceParameters"];

		return data["iceParameters"];
	}
    void processChannelNotifications(std::string event,const json & data)
    {
	    //switch (event)
			//{
				if(event == "icestatechange")
				{
					auto iceState = data["iceState"];// as IceState;

					this->_data["iceState"] = iceState;
        
					this->safeEmit("icestatechange", iceState);

					// Emit observer event.
					this->_observer->safeEmit("icestatechange", iceState);

					//break;
				}else
				if(event == "iceselectedtuplechange")
				{
					auto iceSelectedTuple = data["iceSelectedTuple"];// as TransportTuple;

                    this->_data["iceSelectedTuple"] = iceSelectedTuple;
        
					this->safeEmit("iceselectedtuplechange", iceSelectedTuple);

					// Emit observer event.
					this->_observer->safeEmit("iceselectedtuplechange", iceSelectedTuple);

			//		break;
				}else
				if(event == "dtlsstatechange")
				{
					auto dtlsState = data["dtlsState"];// as DtlsState;
					//auto dtlsRemoteCert = data["dtlsRemoteCert"];// as string;

					this->_data["dtlsState"] = dtlsState;

					if (dtlsState == "connected")
                        this->_data["dtlsRemoteCert"] = data["dtlsRemoteCert"];
         
					  this->safeEmit("dtlsstatechange", dtlsState.get<std::string>());

					// Emit observer event.
					  this->_observer->safeEmit("dtlsstatechange", dtlsState);

		//			break;
				}else
				if(event == "sctpstatechange")
				{
					  auto sctpState = data["sctpState"];// as SctpState;

					  this->_data["sctpState"] = sctpState;
        
		  			this->safeEmit("sctpstatechange", sctpState);

					// Emit observer event.
		  			this->_observer->safeEmit("sctpstatechange", sctpState);

		//			break;
				}else
				if(event == "trace")
				{
					 auto trace = data;// as TransportTraceEventData;
       
		 			 this->safeEmit("trace", trace);

					// Emit observer event.
				 	 this->_observer->safeEmit("trace", trace);

		//			break;
				}else
				{
					//MS_LOGE("ignoring unknown event %s", event.c_str());
				}
			//}
    }
	void handleWorkerNotifications()
	{
		this->_channel->on(this->_internal["transportId"].get<std::string>(),[self = Transport::downcasted_shared_from_this<WebRtcTransport>()]( std::string event,json data )//this->_internal.transportId, (event, data?: any) =>
		{
			self->processChannelNotifications(event,data);
		});
	}
};


}
