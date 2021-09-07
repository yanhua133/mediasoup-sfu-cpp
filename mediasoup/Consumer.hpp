#pragma once
#include <string>
#include <vector>
#include <json.hpp>
#include "Log.hpp"
#include "ChannelAgent.hpp"
#include "PayloadChannelAgent.hpp"
#include "Producer.hpp"
#include "RtpParameters.hpp"
#include "EnhancedEventEmitter.hpp"
#include "Promise.hpp"
using json = nlohmann::json;
using namespace mediasoup;

namespace mediasoup
{
struct ConsumerLayers
{
    /**
     * The spatial layer index (from 0 to N).
     */
    int spatialLayer;

    /**
     * The temporal layer index (from 0 to N).
     */
    int temporalLayer;
    
};
void to_json(json& j, const ConsumerLayers& st) ;

void from_json(const json& j, ConsumerLayers& st) ;

struct ConsumerOptions
{
	/**
	 * The id of the Producer to consume.
	 */
	std::string producerId;

	/**
	 * RTP capabilities of the consuming endpoint.
	 */
	RtpCapabilities rtpCapabilities;

	/**
	 * Whether the Consumer must start in paused mode. Default false.
	 *
	 * When creating a video Consumer, it's recommended to set paused to true,
	 * then transmit the Consumer parameters to the consuming endpoint and, once
	 * the consuming endpoint has created its local side Consumer, unpause the
	 * server side Consumer using the resume() method. This is an optimization
	 * to make it possible for the consuming endpoint to render the video as far
	 * as possible. If the server side Consumer was created with paused: false,
	 * mediasoup will immediately request a key frame to the remote Producer and
	 * suych a key frame may reach the consuming endpoint even before it's ready
	 * to consume it, generating “black” video until the device requests a keyframe
	 * by itself.
	 */
	bool paused = false;

	/**
	 * Preferred spatial and temporal layer for simulcast or SVC media sources.
	 * If unset, the highest ones are selected.
	 */
	ConsumerLayers preferredLayers;

	/**
	 * Whether this Consumer should consume all RTP streams generated by the
	 * Producer.
	 */
	bool pipe = false;

	/**
	 * Custom application data.
	 */
	json appData;
};

/**
 * Valid types for 'trace' event.
 */
//struct ConsumerTraceEventType = 'rtp' | 'keyframe' | 'nack' | 'pli' | 'fir';

/**
 * 'trace' event data.
 */
struct ConsumerTraceEventData
{
	/**
	 * Trace type.
	 */
	std::string type;//: ConsumerTraceEventType;

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
    void to_json(json& j, const ConsumerTraceEventData& st) ;

    void from_json(const json& j, ConsumerTraceEventData& st) ;
struct ConsumerScore
{
	/**
	 * The score of the RTP stream of the consumer.
	 */
	int score;

	/**
	 * The score of the currently selected RTP stream of the producer.
	 */
	int producerScore;

	/**
	 * The scores of all RTP streams in the producer ordered by encoding (just
	 * useful when the producer uses simulcast).
	 */
	std::vector<int> producerScores;
};

    void to_json(json& j, const ConsumerScore& st);

    void from_json(const json& j, ConsumerScore& st) ;

struct ConsumerStat
{
	// Common to all RtpStreams.
	std::string type;
	int timestamp;
	uint32_t ssrc;
	uint32_t rtxSsrc;
	std::string kind;
	std::string mimeType;
	int packetsLost;
	int fractionLost;
	int packetsDiscarded;
	int packetsRetransmitted;
	int packetsRepaired;
	int nackCount;
	int nackPacketCount;
	int pliCount;
	int firCount;
	int score;
	int packetCount;
	int byteCount;
	int bitrate;
	int roundTripTime;
};

/**
 * Consumer type.
 */
//type ConsumerType = 'simple' | 'simulcast' | 'svc' | 'pipe';

//const logger = new Logger('Consumer');

class Consumer  : public EnhancedEventEmitter
{
	// Internal data.
public:
    /*
	struct internal
	{
	    std::string routerId;
		std::string transportId;
		std::string consumerId;
		std::string producerId;
	};*/
    json _internal;
	// Consumer data.
    /*
	struct data:
	{
		std::string kind; //MediaKind
		RtpParameters rtpParameters;
		std::string type;
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

	// Paused flag.
	bool _paused = false;

	// Associated Producer paused flag.
	bool _producerPaused = false;

	// Current priority.
	int _priority = 1;

	// Current score.
	ConsumerScore _score;

	// Preferred layers.
	ConsumerLayers _preferredLayers;

	// Curent layers.
	ConsumerLayers _currentLayers;

	// Observer instance.
    EnhancedEventEmitter *_observer = new EnhancedEventEmitter();
   
	/**
	 * @private
	 * @emits transportclose
	 * @emits producerclose
	 * @emits producerpause
	 * @emits producerresume
	 * @emits score - (score: ConsumerScore)
	 * @emits layerschange - (layers: ConsumerLayers | undefined)
	 * @emits rtp - (packet: Buffer)
	 * @emits trace - (trace: ConsumerTraceEventData)
	 * @emits @close
	 * @emits @producerclose
	 */
	Consumer(
			json& internal,
			json& data,
			std::shared_ptr<ChannelAgent> channel,
			std::shared_ptr<PayloadChannelAgent> payloadChannel,
			json& appData,
			bool paused,
			bool producerPaused,
			ConsumerScore &score,
			ConsumerLayers &preferredLayers
		)
	{

		MS_lOGD("Consumer()");

		this->_internal = internal;
		this->_data = data;
		this->_channel = channel;
		this->_payloadChannel = payloadChannel;
		this->_appData = appData;
		this->_paused = paused;
		this->_producerPaused = producerPaused;
		this->_score = score;
		this->_preferredLayers = preferredLayers;

		this->_handleWorkerNotifications();
	}

	/**
	 * Consumer id.
	 */
	std::string id()
	{
		return this->_internal["consumerId"];
	}

	/**
	 * Associated Producer id.
	 */
	std::string  producerId()
	{
		return this->_internal["producerId"];
	}

	/**
	 * Whether the Consumer is closed.
	 */
	bool closed()
	{
		return this->_closed;
	}

	/**
	 * Media kind.
	 */
	std::string  kind()
	{
		return this->_data["kind"];
	}

	/**
	 * RTP parameters.
	 */
	RtpParameters rtpParameters()
	{
        return this->_data["rtpParameters"];
	}

	/**
	 * Consumer type.
	 */
	std::string type()
	{
		return this->_data["type"];
	}

	/**
	 * Whether the Consumer is paused.
	 */
	bool paused()
	{
		return this->_paused;
	}

	/**
	 * Whether the associate Producer is paused.
	 */
	bool producerPaused()
	{
		return this->_producerPaused;
	}

	/**
	 * Current priority.
	 */
	int priority()
	{
		return this->_priority;
	}

	/**
	 * Consumer score.
	 */
	ConsumerScore score()
	{
		return this->_score;
	}

	/**
	 * Preferred video layers.
	 */
	ConsumerLayers preferredLayers()
	{
		return this->_preferredLayers;
	}

	/**
	 * Current video layers.
	 */
	ConsumerLayers currentLayers()
	{
		return this->_currentLayers;
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
	void appData(json appData) // eslint-disable-line no-unused-vars
	{
		MS_THROW_lOG("cannot override appData object");
	}

	/**
	 * Observer.
	 *
	 * @emits close
	 * @emits pause
	 * @emits resume
	 * @emits score - (score: ConsumerScore)
	 * @emits layerschange - (layers: ConsumerLayers | undefined)
	 * @emits trace - (trace: ConsumerTraceEventData)
	 */
	EnhancedEventEmitter * observer()
	{
		return this->_observer;
	}

	/**
	 * Close the Consumer.
	 */
	void close()
	{
		if (this->_closed)
			return;

		MS_lOGD("close()");

		this->_closed = true;

		// Remove notification subscriptions.
		this->_channel->removeAllListeners(this->_internal["consumerId"]);
		this->_payloadChannel->removeAllListeners(this->_internal["consumerId"]);

		this->_channel->request("consumer.close", this->_internal);

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
		this->_channel->removeAllListeners(this->_internal["consumerId"]);
		this->_payloadChannel->removeAllListeners(this->_internal["consumerId"]);

		this->safeEmit("transportclose");

		// Emit observer event.
		this->_observer->safeEmit("close");
	}

	/**
	 * Dump Consumer.
	 */
	json dump() //Promise<any>
	{
		MS_lOGD("dump()");

		return this->_channel->request("consumer.dump", this->_internal);
	}

	/**
	 * Get Consumer stats.
	 */
	json getStats() //Promise<Array<ConsumerStat | ProducerStat>> 
	{
		MS_lOGD("getStats()");

		return this->_channel->request("consumer.getStats", this->_internal);
	}

	/**
	 * Pause the Consumer.
	 */
	void pause() //Promise<void>
	{
		MS_lOGD("pause()");

		const bool wasPaused = this->_paused || this->_producerPaused;

		this->_channel->request("consumer.pause", this->_internal);

		this->_paused = true;

		// Emit observer event.
		if (!wasPaused)
			this->_observer->safeEmit("pause");
	}

	/**
	 * Resume the Consumer.
	 */
	void resume() //Promise<void>
	{
		MS_lOGD("resume()");

		const bool wasPaused = this->_paused || this->_producerPaused;

		this->_channel->request("consumer.resume", this->_internal);

		this->_paused = false;

		// Emit observer event.
		if (wasPaused && !this->_producerPaused)
			this->_observer->safeEmit("resume");
	}

	/**
	 * Set preferred video layers.
	 */
	void setPreferredLayers( 
			int spatialLayer,
			int temporalLayer
	)
	{
		MS_lOGD("setPreferredLayers()");

        const json reqData = {
            {"spatialLayer",spatialLayer},
            {"temporalLayer",temporalLayer}
        };

		json data = this->_channel->request(
			"consumer.setPreferredLayers", this->_internal, reqData);

		this->_preferredLayers = data;
	}

	/**
	 * Set priority.
	 */
	void setPriority(int priority) //Promise<void>
	{
		MS_lOGD("setPriority()");

		const json reqData = {
            {"priority",priority }
       };

        json data = this->_channel->request(
			"consumer.setPriority", this->_internal, reqData);

		this->_priority = data["priority"].get<int>();
	}

	/**
	 * Unset priority.
	 */
	void unsetPriority() //Promise<void> 
	{
		MS_lOGD("unsetPriority()");

		const json reqData = {
            {"priority",1 }
       };

        json data = this->_channel->request(
			"consumer.setPriority", this->_internal, reqData);

        this->_priority = data["priority"].get<int>();
	}

	/**
	 * Request a key frame to the Producer.
	 */
	void requestKeyFrame() //Promise<void>
	{
		MS_lOGD("requestKeyFrame()");

		this->_channel->request("consumer.requestKeyFrame", this->_internal);
	}

	/**
	 * Enable 'trace' event.
	 */
	void  enableTraceEvent(std::vector<std::string> types) //Promise<void>
	{
		MS_lOGD("enableTraceEvent()");

		const json reqData = {
            { "types",types }
       };

		this->_channel->request(
			"consumer.enableTraceEvent", this->_internal, reqData);
	}
  void processChannelNotifications(std::string event,const json & data) 
  {
//switch (event)
			//{
				if(event == "producerclose")
				{
					if (this->_closed)
						return;

					this->_closed = true;

					// Remove notification subscriptions.
					this->_channel->removeAllListeners(this->_internal["consumerId"]);
					this->_payloadChannel->removeAllListeners(this->_internal["consumerId"]);

					this->emit("@producerclose");
					this->safeEmit("producerclose");

					// Emit observer event.
					this->_observer->safeEmit("close");

					//return;
				} else 
				if(event == "producerpause")
				{
					if (this->_producerPaused)
					return;

					const bool wasPaused = this->_paused || this->_producerPaused;

					this->_producerPaused = true;

					this->safeEmit("producerpause");

					// Emit observer event.
					if (!wasPaused)
						this->_observer->safeEmit("pause");

				//	return;
				}else
				if(event == "producerresume")
				{
					if (!this->_producerPaused)
						return;

					const bool wasPaused = this->_paused || this->_producerPaused;

					this->_producerPaused = false;

					this->safeEmit("producerresume");

					// Emit observer event.
					if (wasPaused && !this->_paused)
						this->_observer->safeEmit("resume");

					//return;
				}else
				if(event == "score")
				{
                    ConsumerScore score = data;

					this->_score = score;


					this->safeEmit("score", score);

					// Emit observer event.
					this->_observer->safeEmit("score", score);

					//return;
				}else
				if(event == "layerschange")
				{
                    ConsumerLayers layers = data;// as ConsumerLayers | undefined;

					this->_currentLayers = layers;

					this->safeEmit("layerschange", layers);

					// Emit observer event.
					this->_observer->safeEmit("layerschange", layers);

				//	return;
				}else
				if(event == "trace")
				{
                    ConsumerTraceEventData trace = data ;//as ConsumerTraceEventData;

					this->safeEmit("trace", trace);

					// Emit observer event.
					this->_observer->safeEmit("trace", trace);

				//	return;
				}else
				{
					MS_lOGE("ignoring unknown event %s", event.c_str());
				}
			//}
  }
  void processPayloadChannelNotifications(std::string event,const json & data) 
  {
    //switch (event)
				//{
					if(event == "rtp")
					{
						if (this->_closed)
							return;

					//	const packet = payload;

					//	this->safeEmit("rtp", packet);

						//break;
					}else
					{
						MS_lOGE("ignoring unknown event %s", event.c_str());
					}
				//}
  }
  void	_handleWorkerNotifications()
	{
     //this->_internal["consumerId"], (event, data?: any) =>
		this->_channel->on(this->_internal["consumerId"],[&]( std::string event,json data )
		{
         
			processChannelNotifications(event,data);
		});
        //this->_internal["consumerId"],
    //    (event, data: any | undefined, payload: Buffer) =>
		this->_payloadChannel->on(this->_internal["consumerId"],[&]( std::string event,json data )
        {
           
            processPayloadChannelNotifications(event,data);
        });
	}
};
}
