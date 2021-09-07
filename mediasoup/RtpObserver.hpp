#pragma once
#include "Log.hpp"
#include "EnhancedEventEmitter.hpp"
#include "ChannelAgent.hpp"
#include "PayloadChannelAgent.hpp"
#include "Producer.hpp"
#include "json.hpp"
#include "Promise.hpp"
using json = nlohmann::json;
using namespace mediasoup;

//const logger = new Logger('RtpObserver');

namespace mediasoup
{
  
struct RtpObserverAddRemoveProducerOptions
{
	/**
	 * The id of the Producer to be added or removed.
	 */
	std::string producerId;
};
struct RtpObserverParams
{
    json internal;
    std::shared_ptr<ChannelAgent> channel;
    std::shared_ptr<PayloadChannelAgent> payloadChannel;
    json appData;
    std::function<std::shared_ptr<Producer>(std::string&)> getProducerById;
};

class RtpObserver : public  EnhancedEventEmitter
{
	// Internal data.
public:
/*
	 _internal:
	{
		routerId: string;
		rtpObserverId: string;
	};
*/
  json _internal;
	// Channel instance.
	std::shared_ptr<ChannelAgent>  _channel;

	// PayloadChannelAgent instance.
	std::shared_ptr<PayloadChannelAgent>  _payloadChannel;

	// Closed flag.
	bool _closed = false;

	// Paused flag.
	bool _paused = false;

	// Custom app data.
	json _appData;

	// Method to retrieve a Producer.
    std::function<std::shared_ptr<Producer>(std::string&)> _getProducerById;//: (producerId: string) => Producer;

	// Observer instance.
	EnhancedEventEmitter * _observer = new EnhancedEventEmitter();

	/**
	 * @private
	 * @interface
	 * @emits routerclose
	 * @emits @close
	 */
	RtpObserver(
                RtpObserverParams &params
		//{
//			json &internal,
//			std::shared_ptr<ChannelAgent> channel,
//			std::shared_ptr<PayloadChannelAgent> payloadChannel,
//			json &appData,
//			getProducerById __getProducerById
	//	}:
	//	{
	//		internal: any;
	//		channel: Channel;
	//		payloadChannel: PayloadChannelAgent;
	//		appData: any;
	//		getProducerById: (producerId: string) => Producer;
//		}
	)
	{
		//super();

		MS_lOGD("RtpObserver()");

		this->_internal = params.internal;
		this->_channel = params.channel;
		this->_payloadChannel = params.payloadChannel;
		this->_appData = params.appData;
		this->_getProducerById = params.getProducerById;

	}

	/**
	 * RtpObserver id.
	 */
	std::string id()
	{
		return this->_internal["rtpObserverId"];
	}

	/**
	 * Whether the RtpObserver is closed.
	 */
	bool closed()
	{
		return this->_closed;
	}

	/**
	 * Whether the RtpObserver is paused.
	 */
	bool paused()
	{
		return this->_paused;
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
	void setAppData(json &appData) // eslint-disable-line no-unused-vars
	{
		MS_THROW_lOG("cannot override appData object");
	}

	/**
	 * Observer.
	 *
	 * @emits close
	 * @emits pause
	 * @emits resume
	 * @emits addproducer - (producer: Producer)
	 * @emits removeproducer - (producer: Producer)
	 */
	EnhancedEventEmitter *observer()
	{
		return this->_observer;
	}

	/**
	 * Close the RtpObserver.
	 */
	void close()
	{
		if (this->_closed)
			return;

		MS_lOGD("close()");

		this->_closed = true;

		// Remove notification subscriptions.
		this->_channel->removeAllListeners(this->_internal["rtpObserverId"]);
		this->_payloadChannel->removeAllListeners(this->_internal["rtpObserverId"]);

		this->_channel->request("rtpObserver.close", this->_internal);
			//.catch(() => {});

		this->emit("@close");

		// Emit observer event.
		this->_observer->safeEmit("close");
	}

	/**
	 * Router was closed.
	 *
	 * @private
	 */
	void routerClosed()
	{
		if (this->_closed)
			return;

		MS_lOGD("routerClosed()");

		this->_closed = true;

		// Remove notification subscriptions.
		this->_channel->removeAllListeners(this->_internal["rtpObserverId"]);
		this->_payloadChannel->removeAllListeners(this->_internal["rtpObserverId"]);

		this->safeEmit("routerclose");

		// Emit observer event.
		this->_observer->safeEmit("close");
	}

	/**
	 * Pause the RtpObserver.
	 */
	json pause() //: Promise<void>
	{
		MS_lOGD("pause()");

		const bool wasPaused = this->_paused;

		this->_channel->request("rtpObserver.pause", this->_internal);

		this->_paused = true;

		// Emit observer event.
		if (!wasPaused)
			this->_observer->safeEmit("pause");
	}

	/**
	 * Resume the RtpObserver.
	 */
	json resume()//: Promise<void>
	{
		MS_lOGD("resume()");

		const bool wasPaused = this->_paused;

		this->_channel->request("rtpObserver.resume", this->_internal);

		this->_paused = false;

		// Emit observer event.
		if (wasPaused)
			this->_observer->safeEmit("resume");
	}

	/**
	 * Add a Producer to the RtpObserver.
	 */
	void addProducer(std::string producerId)// }: RtpObserverAddRemoveProducerOptions): Promise<void>
	{
		MS_lOGD("addProducer()");

		auto producer = this->_getProducerById(producerId);
		json internal = this->_internal;//{ ...this->_internal, producerId };
        internal["producerId"] = producerId;
		this->_channel->request("rtpObserver.addProducer", internal);

		// Emit observer event.
		this->_observer->safeEmit("addproducer", producer);
	}

	/**
	 * Remove a Producer from the RtpObserver.
	 */
	json removeProducer(std::string producerId)// }: RtpObserverAddRemoveProducerOptions): Promise<void>
	{
		MS_lOGD("removeProducer()");

		auto producer = this->_getProducerById(producerId);
		json internal = this->_internal;//{ ...this->_internal, producerId };
        internal["producerId"] = producerId;

		this->_channel->request("rtpObserver.removeProducer", internal);

		// Emit observer event.
		this->_observer->safeEmit("removeproducer", producer);
	}
};


}
