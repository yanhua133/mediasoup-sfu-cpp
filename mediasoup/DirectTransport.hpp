#pragma once
#include <string>
#include <vector>
#include <json.hpp>
#include "Log.hpp"
#include "Transport.hpp"
#include "EnhancedEventEmitter.hpp"
#include "Promise.hpp"
using json = nlohmann::json;
using namespace mediasoup;

namespace mediasoup
{
  
struct DirectTransportOptions
{
	/**
	 * Maximum allowed size for direct messages sent from DataProducers.
	 * Default 262144.
	 */
	int maxMessageSize = 262144;

	/**
	 * Custom application data.
	 */
	json _appData;
};

struct DirectTransportStat
{
	// Common to all Transports.
	std::string type;
	std::string transportId;
	int timestamp;
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
};

//const logger = new Logger('DirectTransport');

class DirectTransport : public Transport
{

public:
	// DirectTransport data.
    json _data;
	
	/**
	 * @private
	 * @emits rtcp - (packet: Buffer)
	 * @emits trace - (trace: TransportTraceEventData)
	 */
	DirectTransport(TransportParams &params) : Transport(params)
	{
		//super(params);

		MS_lOGD("DirectTransport()");

		this->_data =
		{
			// Nothing for now.
		};

		//this->_handleWorkerNotifications();
	}

	/**
	 * Observer.
	 *
	 * @override
	 * @emits close
	 * @emits newdataproducer - (dataProducer: DataProducer)
	 * @emits newdataconsumer - (dataProducer: DataProducer)
	 * @emits trace - (trace: TransportTraceEventData)
	 */
	EnhancedEventEmitter* observer()
	{
		return this->_observer;
	}

	/**
	 * Close the DirectTransport.
	 *
	 * @override
	 */
	void close()
	{
		if (this->_closed)
			return;

		//super.close();
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

		//super.routerClosed();
	}

	/**
	 * Get DirectTransport stats.
	 *
	 * @override
	 */
	json getStats()// Promise<DirectTransportStat[]>
	{
		MS_lOGD("getStats()");

		return this->_channel->request("transport.getStats", this->_internal);
	}

	/**
	 * NO-OP method in DirectTransport.
	 *
	 * @override
	 */
    virtual void  connect(json *params) // Promise<void>
	{
		MS_lOGD("connect()");
	}

	/**
	 * @override
	 */
	// eslint-disable-next-line @typescript-eslint/no-unused-vars
	json setMaxIncomingBitrate(int bitrate) // Promise<void>
	{
		//throw new UnsupportedError(
	//		'setMaxIncomingBitrate() not implemented in DirectTransport');
	}

	/**
	 * Send RTCP packet.
	 */
	void sendRtcp(uint8_t* rtcpPacket)
	{
		//if (!Buffer.isBuffer(rtcpPacket))
		//{
		//	MS_THROW_lOG("rtcpPacket must be a Buffer");
		//}

		//this->_payloadChannel->notify(
		//	'transport.sendRtcp', this->_internal, undefined, rtcpPacket);
	}
  void processChannelNotifications(std::string event,const json & data) 
  {
    	//switch (event)
		//	{
				if(event == "trace")
				{
					auto trace = data;// as TransportTraceEventData;

					this->safeEmit("trace", trace);

					// Emit observer event.
					this->_observer->safeEmit("trace", trace);

				//	break;
				} else 
				{
					//MS_lOGE('ignoring unknown event "%s"', event.c_str());
				}
		//	}
  }
  void processPayloadChannelNotifications(std::string event,const json & data) 
  {
    	//switch (event)
			//	{
					if(event == "rtcp")
					{
						if (this->_closed)
							return;

						//const packet = payload;

						//this->safeEmit("rtcp", packet);

					//break;
					}else
					{
						//MS_lOGE('ignoring unknown event "%s"', event.c_str());
					}
			//	}
  }
	void handleWorkerNotifications()
	{
		this->_channel->on(this->_internal["transportId"].get<std::string>(),[self = Transport::downcasted_shared_from_this<DirectTransport>()]( std::string event,json data ) //this->_internal.transportId, (event, data?: any) =>
		{		
			self->processChannelNotifications(event,data);
		});

		this->_payloadChannel->on(this->_internal["transportId"].get<std::string>(),[self = Transport::downcasted_shared_from_this<DirectTransport>()]( std::string event,json data ) //
		//	this->_internal.transportId,
		//	(event, data: any | undefined, payload: Buffer) =>
			{			     
            self->processPayloadChannelNotifications(event,data);
			});
	}
};


}
