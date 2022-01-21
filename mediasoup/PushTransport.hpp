#pragma once
#include "Log.hpp"
#include "EnhancedEventEmitter.hpp"
#include "Transport.hpp"
#include "json.hpp"

#include "Promise.hpp"
using json = nlohmann::json;
using namespace mediasoup;


namespace mediasoup
{
  
struct PushTransportOptions
{
	/**
	 * Custom application data.
	 */
	json appData;
};

void to_json(json& j, const PushTransportOptions& st);
void from_json(const json& j, PushTransportOptions& st);
//const logger = new Logger('PushTransport');

class PushTransport : public Transport
{
public:   
    json _data;

	PushTransport(TransportParams &params): Transport(params)
	{
        //Transport::Transport(params);

		MS_lOGD("PushTransport()");		

		//this->_handleWorkerNotifications();
	}

	/**
	 * Observer.
	 *
	 * @override
	 * @emits close
	 * @emits newproducer - (producer: Producer)
	 * @emits newconsumer - (consumer: Consumer)
	 * @emits trace - (trace: TransportTraceEventData)
	 */
	EnhancedEventEmitter *observer()
	{
		return this->_observer;
	}

	/**
	 * Close the PushTransport.
	 *
	 * @override
	 */
	void close()
	{
		if (this->_closed)
			return;

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

		//super.routerClosed();
		Transport::routerClosed();
	}

	/**
	 * PushTransport stats.
	 *
	 * @override
	 */
	json getStats()//: Promise<PushTransportStat[]>
	{
		MS_lOGD("getStats()");

		return this->_channel->request("transport.getStats", this->_internal);
	}

	/**
	 * Provide the PushTransport remote parameters.
	 *
	 * @override
	 */
    virtual void connect(json & params)
	{
		MS_lOGD("connect()");

        const json reqData;

		json data =
			this->_channel->request("transport.connect", this->_internal, reqData);

	}

    void processChannelNotifications(std::string event,const json & data)
    {
		if (event == "trace")
		{
			auto trace = data;// as TransportTraceEventData;

			this->safeEmit("trace", trace);

			// Emit observer event.
			this->_observer->safeEmit("trace", trace);

		}
		else
		{
			//MS_LOGE("ignoring unknown event %s", event.c_str());
		}
    }
	void handleWorkerNotifications()
	{
		this->_channel->on(this->_internal["transportId"].get<std::string>(),[self = Transport::downcasted_shared_from_this<PushTransport>()]( std::string event,json data )//this->_internal.transportId, (event, data?: any) =>
		{
			self->processChannelNotifications(event,data);
		});
	}
};


}
