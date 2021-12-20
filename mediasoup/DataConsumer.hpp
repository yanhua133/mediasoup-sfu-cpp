#pragma once
#include <string>
#include <vector>
#include <json.hpp>
#include "Log.hpp"
#include "ChannelAgent.hpp"
#include "PayloadChannelAgent.hpp"
#include "Producer.hpp"
#include "SctpParameters.hpp"
#include "EnhancedEventEmitter.hpp"
#include "Promise.hpp"
using json = nlohmann::json;
using namespace mediasoup;

namespace mediasoup
{

struct DataConsumerOptions
{
	/**
	 * The id of the DataProducer to consume.
	 */
	std::string dataProducerId;

	/**
	 * Just if consuming over SCTP.
	 * Whether data messages must be received in order. If true the messages will
	 * be sent reliably. Defaults to the value in the DataProducer if it has type
	 * 'sctp' or to true if it has type 'direct'.
	 */
	bool ordered;

	/**
	 * Just if consuming over SCTP.
	 * When ordered is false indicates the time (in milliseconds) after which a
	 * SCTP packet will stop being retransmitted. Defaults to the value in the
	 * DataProducer if it has type 'sctp' or unset if it has type 'direct'.
	 */
	int maxPacketLifeTime;

	/**
	 * Just if consuming over SCTP.
	 * When ordered is false indicates the maximum number of times a packet will
	 * be retransmitted. Defaults to the value in the DataProducer if it has type
	 * 'sctp' or unset if it has type 'direct'.
	 */
	int maxRetransmits;

	/**
	 * Custom application data.
	 */
	json _appData;
};

struct DataConsumerStat
{
	std::string type;
	int timestamp;
	std::string label;
	std::string protocol;
	int messagesSent;
	int bytesSent;
};

/**
 * DataConsumer type.
 */
//type DataConsumerType = 'sctp' | 'direct';

//const logger = new Logger('DataConsumer');

class DataConsumer   : public EnhancedEventEmitter, public std::enable_shared_from_this<DataConsumer>
{

public:
	// Internal data.
    /*
	struct internal:
	{
		std::string routerId;
		std::string transportId;
		std::string dataProducerId;
		std::string dataConsumerId;
	};*/
  json _internal;
	// DataConsumer data.
    /*
	data _data:
	{
		std::string type;//: DataConsumerType;
		sctpStreamParameters?: SctpStreamParameters;
		std::string label;
		std::string protocol;
	};*/
  json _data;
	// Channel instance.
	std::shared_ptr<ChannelAgent> _channel;

	// PayloadChannelAgent instance.
	std::shared_ptr<PayloadChannelAgent> _payloadChannel;

	// Closed flag.
	bool _closed = false;

	// Custom app data.
	 json _appData;

	// Observer instance.
    EnhancedEventEmitter * _observer = new EnhancedEventEmitter();

	/**
	 * @private
	 * @emits transportclose
	 * @emits dataproducerclose
	 * @emits message - (message: Buffer, ppid)
	 * @emits sctpsendbufferfull
	 * @emits bufferedamountlow - (bufferedAmount)
	 * @emits @close
	 * @emits @dataproducerclose
	 */
	DataConsumer(
    	json& internal,
			json& data,
			std::shared_ptr<ChannelAgent> channel,
			std::shared_ptr<PayloadChannelAgent> payloadChannel,
			json& appData
	)
	{
		//super();

		MS_lOGD("DataConsumer()");
		
		this->_internal = internal;
		this->_data = data;
		this->_channel = channel;
		this->_payloadChannel = payloadChannel;
		this->_appData = appData;

		//this->_handleWorkerNotifications();
	}

	/**
	 * DataConsumer id.
	 */
	std::string id()
	{
		return this->_internal["dataConsumerId"];
	}

	/**
	 * Associated DataProducer id.
	 */
	std::string dataProducerId()
	{
		return this->_internal["dataProducerId"];
	}

	/**
	 * Whether the DataConsumer is closed.
	 */
	bool closed()
	{
		return this->_closed;
	}

	/**
	 * DataConsumer type.
	 */
	std::string type()//: DataConsumerType
	{
		return this->_data["type"];
	}

	/**
	 * SCTP stream parameters.
	 */
	SctpStreamParameters sctpStreamParameters()
	{
		return this->_data["sctpStreamParameters"];
	}

	/**
	 * DataChannel label.
	 */
	std::string label()
	{
		return this->_data["label"];
	}

	/**
	 * DataChannel protocol.
	 */
	std::string protocol()
	{
		return this->_data["protocol"];
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
	 * Observer.
	 *
	 * @emits close
	 */
	EnhancedEventEmitter* observer()
	{
		return this->_observer;
	}

	/**
	 * Close the DataConsumer.
	 */
	void close()
	{
		if (this->_closed)
			return;

		MS_lOGD("close()");

		this->_closed = true;

		// Remove notification subscriptions.
		this->_channel->removeAllListeners(this->_internal["dataConsumerId"]);
		this->_payloadChannel->removeAllListeners(this->_internal["dataConsumerId"]);

		this->_channel->request("dataConsumer.close", this->_internal);

		this->emit("@close");

		// Emit observer event.
		this->_observer->safeEmit("close");
	}
	/**
	 * Transport was closed.
	 *
	 * @private
	 */
	void transportClosed()
	{
		if (this->_closed)
			return;

		MS_lOGD("transportClosed()");

		this->_closed = true;

		// Remove notification subscriptions.
		this->_channel->removeAllListeners(this->_internal["dataConsumerId"]);
		this->_payloadChannel->removeAllListeners(this->_internal["dataConsumerId"]);

		this->safeEmit("transportclose");

		// Emit observer event.
		this->_observer->safeEmit("close");
	}

	/**
	 * Dump DataConsumer.
	 */
	json dump() // Promise<any>
	{
		MS_lOGD("dump()");

		return this->_channel->request("dataConsumer.dump", this->_internal);
	}

	/**
	 * DataConsumer stats.
	 */
	json getStats() //: Promise<DataConsumerStat[]>
	{
		MS_lOGD("getStats()");

		return this->_channel->request("dataConsumer.getStats", this->_internal);
	}

	/**
	 * Set buffered amount low threshold.
	 */
	json  setBufferedAmountLowThreshold(int threshold)//: Promise<void>
	{
		MS_lOGD("setBufferedAmountLowThreshold() [threshold:%d]", threshold);

		const json reqData = {
            { "threshold",threshold }
      };

		this->_channel->request(
			"dataConsumer.setBufferedAmountLowThreshold", this->_internal, reqData);
	}

	/**
	 * Send data.
	 */
	json send(std::string message, int ppid)//: Promise<void>
	{
		//if (typeof message !== 'string' && !Buffer.isBuffer(message))
		//{
		//	MS_THROW_lOG("message must be a string or a Buffer");
		//}

		/*
		 * +-------------------------------+----------+
		 * | Value                         | SCTP     |
		 * |                               | PPID     |
		 * +-------------------------------+----------+
		 * | WebRTC String                 | 51       |
		 * | WebRTC Binary Partial         | 52       |
		 * | (Deprecated)                  |          |
		 * | WebRTC Binary                 | 53       |
		 * | WebRTC String Partial         | 54       |
		 * | (Deprecated)                  |          |
		 * | WebRTC String Empty           | 56       |
		 * | WebRTC Binary Empty           | 57       |
		 * +-------------------------------+----------+
		 */

		//if (typeof ppid !== 'number')
		//{
		//	ppid = (typeof message === 'string')
		//		? message.length > 0 ? 51 : 56
		//		: message.length > 0 ? 53 : 57;
		//}

		// Ensure we honor PPIDs.
		//if (ppid === 56)
		//	message = ' ';
		//else if (ppid === 57)
		//	message = Buffer.alloc(1);

		const json requestData = {
          { "ppid",ppid }
        };

		//this->_payloadChannel->request(
		//	"dataConsumer.send", this->_internal, requestData, message);
	}

	/**
	 * buffered amount size.
	 */
	json getBufferedAmount()//: Promise<number>
	{
		MS_lOGD("getBufferedAmount()");

		json data =
			this->_channel->request("dataConsumer.getBufferedAmount", this->_internal);

        return data;//bufferedAmount;
	}
  void processChannelNotifications(std::string event,const json & data)
  {
    //switch (event)
			//{
				if(event == "dataproducerclose")
				{
					if (this->_closed)
						return;

					this->_closed = true;

					// Remove notification subscriptions.
					this->_channel->removeAllListeners(this->_internal["dataConsumerId"]);
					this->_payloadChannel->removeAllListeners(this->_internal["dataConsumerId"]);

					this->emit("@dataproducerclose");
					this->safeEmit("dataproducerclose");

					// Emit observer event.
					this->_observer->safeEmit("close");

				//	break;
				}else
				if(event == "sctpsendbufferfull")
				{
					this->safeEmit("sctpsendbufferfull");

				//	break;
				}else
				if(event == "bufferedamountlow")
				{
					auto  bufferedAmount  = data;// as { bufferedAmount };

					this->safeEmit("bufferedamountlow", bufferedAmount);

					//break;
				} else
				{
					MS_lOGE("ignoring unknown event %s in channel listener", event.c_str());
				}
			//}
  }
  void processPayloadChannelNotifications(std::string event,const json & data)
  {
    //switch (event)
			//	{
					if(event == "message")
					{
						if (this->_closed)
							return;

						//const ppid = data.ppid as number;
						//const message = payload;

						//this->safeEmit("message", message, ppid);

						//break;
					} else
					{
						MS_lOGE("ignoring unknown event %s in payload channel listener", event.c_str());
					}
			//	}
  }
	void handleWorkerNotifications()
	{
		this->_channel->on(this->_internal["dataConsumerId"].get<std::string>(),[self = shared_from_this()]( std::string event,json data ) //this->_internal["dataConsumerId"], (event, data: any) =>
		{     
			self->processChannelNotifications(event,data);
			
		});

		this->_payloadChannel->on(this->_internal["dataConsumerId"].get<std::string>(),[self = shared_from_this()]( std::string event,json data ) //
			//this->_internal["dataConsumerId"],
			//(event, data: any | undefined, payload: Buffer) =>
			{
			   
            self->processPayloadChannelNotifications(event,data);
			});
	}
};


}
