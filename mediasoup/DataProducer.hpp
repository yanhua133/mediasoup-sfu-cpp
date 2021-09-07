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
  
struct DataProducerOptions
{
	/**
	 * DataProducer id (just for Router.pipeToRouter() method).
	 */
	std::string id;

	/**
	 * SCTP parameters defining how the endpoint is sending the data.
	 * Just if messages are sent over SCTP.
	 */
    SctpStreamParameters sctpStreamParameters;

	/**
	 * A label which can be used to distinguish this DataChannel from others.
	 */
	std::string label;

	/**
	 * Name of the sub-protocol used by this DataChannel.
	 */
	std::string protocol;

	/**
	 * Custom application data.
	 */
	json _appData;
};

struct DataProducerStat
{
	std::string type;
	int timestamp;
	std::string label;
	std::string protocol;
	int messagesReceived;
	int bytesReceived;
};

/**
 * DataProducer type.
 */
// type DataProducerType = 'sctp' | 'direct';

//const logger = new Logger('DataProducer');

class DataProducer   : public EnhancedEventEmitter
{

public:
	// Internal data.
    /*
	struct  internal:
	{
		std::string routerId;
		std::string transportId;
		std::string dataProducerId;
	};*/
    json _internal;
	// DataProducer data.
    /*
	struct data:
	{
		std::string type ;//: DataProducerType;
		SctpStreamParameters sctpStreamParameters;
		std::string label;
		std::string protocol;
	};*/
    json _data;
	// Channel instance.
	std::shared_ptr<ChannelAgent> _channel;//: Channel;

	// PayloadChannelAgent instance.
	std::shared_ptr<PayloadChannelAgent> _payloadChannel;//: PayloadChannelAgent;

	// Closed flag.
	bool _closed = false;

	// Custom app data.
	json _appData;

	// Observer instance.
    EnhancedEventEmitter * _observer = new EnhancedEventEmitter();

	/**
	 * @private
	 * @emits transportclose
	 * @emits @close
	 */
	DataProducer(
		  json& internal,
			json& data,
			std::shared_ptr<ChannelAgent> channel,
			std::shared_ptr<PayloadChannelAgent> payloadChannel,
			json &appData
	)
	{
		//super();

		MS_lOGD("DataProducer()");

		this->_internal = internal;
		this->_data = data;
		this->_channel = channel;
		this->_payloadChannel = payloadChannel;
		this->_appData = appData;

		this->_handleWorkerNotifications();
	}

	/**
	 * DataProducer id.
	 */
	std::string id()
	{
		return this->_internal["dataProducerId"];
	}

	/**
	 * Whether the DataProducer is closed.
	 */
	bool closed()
	{
		return this->_closed;
	}

	/**
	 * DataProducer type.
	 */
	std::string type() // DataProducerType
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
	 * Close the DataProducer.
	 */
	void close()
	{
		if (this->_closed)
			return;

		MS_lOGD("close()");

		this->_closed = true;

		// Remove notification subscriptions.
		this->_channel->removeAllListeners(this->_internal["dataProducerId"]);
		this->_payloadChannel->removeAllListeners(this->_internal["dataProducerId"]);

		this->_channel->request("dataProducer.close", this->_internal);

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
		this->_channel->removeAllListeners(this->_internal["dataProducerId"]);
		this->_payloadChannel->removeAllListeners(this->_internal["dataProducerId"]);

		this->safeEmit("transportclose");

		// Emit observer event.
		this->_observer->safeEmit("close");
	}

	/**
	 * Dump DataProducer.
	 */
	json dump()// Promise<any>
	{
		MS_lOGD("dump()");

		return this->_channel->request("dataProducer.dump", this->_internal);
	}

	/**
	 * DataProducer stats.
	 */
	json getStats()//: Promise<DataProducerStat[]>
	{
		MS_lOGD("getStats()");

		return this->_channel->request("dataProducer.getStats", this->_internal);
	}

	/**
	 * Send data (just valid for DataProducers created on a DirectTransport).
	 */
	void send(std::string message, int ppid)
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

	//	if (typeof ppid !== 'number')
	//	{
	//		ppid = (typeof message === 'string')
	//			? message.length > 0 ? 51 : 56
	//			: message.length > 0 ? 53 : 57;
	//	}

		// Ensure we honor PPIDs.
		//if (ppid === 56)
	//		message = " ";
	//	else if (ppid === 57)
		//	message = Buffer.alloc(1);

		//const json notifData = { "ppid":ppid };

		//this->_payloadChannel->notify(
		//	'dataProducer.send', this->_internal, notifData, message);
	}
  void processChannelNotifications(std::string event,const json & data) 
  {
  }
  void processPayloadChannelNotifications(std::string event,const json & data) 
  {
  }
  void _handleWorkerNotifications()
  {
		// No need to subscribe to any event.
  }
};


}
