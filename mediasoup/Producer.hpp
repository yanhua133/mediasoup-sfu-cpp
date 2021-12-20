
#pragma once
#include <string>
#include <vector>
#include <json.hpp>
#include "Log.hpp"
#include "ChannelAgent.hpp"
#include "PayloadChannelAgent.hpp"
#include "RtpParameters.hpp"
#include "EnhancedEventEmitter.hpp"
#include "Promise.hpp"
using json = nlohmann::json;
using namespace mediasoup;

namespace mediasoup
{
  
struct ProducerOptions
{
	/**
	 * Producer id (just for Router.pipeToRouter() method).
	 */
	std::string id;

	/**
	 * Media kind ('audio' or 'video').
	 */
    std::string  kind;

	/**
	 * RTP parameters defining what the endpoint is sending.
	 */
    RtpParameters rtpParameters;

	/**
	 * Whether the producer must start in paused mode. Default false.
	 */
	bool paused = false;

	/**
	 * Just for video. Time (in ms) before asking the sender for a new key frame
	 * after having asked a previous one. Default 0.
	 */
	int keyFrameRequestDelay;

	/**
	 * Custom application data.
	 */
	json appData;
};

/**
 * Valid types for 'trace' event.
 */
//type ProducerTraceEventType = 'rtp' | 'keyframe' | 'nack' | 'pli' | 'fir';

/**
 * 'trace' event data.
 */
struct ProducerTraceEventData
{
	/**
	 * Trace type.
	 */
	std::string type ;// ProducerTraceEventType;

	/**
	 * Event timestamp.
	 */
	int timestamp;

	/**
	 * Event direction.
	 */
	std::string direction ;// 'in' | 'out';

	/**
	 * Per type information.
	 */
	std::string info;//: any;
};
  void from_json(const json& j, ProducerTraceEventData& st);
  void to_json(json& j, ProducerTraceEventData& st);
struct ProducerScore
{
	/**
	 * SSRC of the RTP stream.
	 */
	int ssrc;

	/**
	 * RID of the RTP stream.
	 */
	std::string rid;

	/**
	 * The score of the RTP stream.
	 */
	int score;
};
void from_json(const json& j, ProducerScore& st);
void to_json(json& j, const ProducerScore& st);
struct ProducerVideoOrientation
{
	/**
	 * Whether the source is a video camera.
	 */
	bool camera;

	/**
	 * Whether the video source is flipped.
	 */
	bool flip;

	/**
	 * Rotation degrees (0, 90, 180 or 270).
	 */
	int rotation;
};
  void from_json(const nlohmann::json& j, ProducerVideoOrientation& st);
  void to_json(json& j, const ProducerVideoOrientation& st) ;
struct ProducerStat
{
	// Common to all RtpStreams.
	std::string type;
	int timestamp;
	uint32_t ssrc;
	uint32_t rtxSsrc;
	std::string rid;
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
	// RtpStreamRecv specific.
	int jitter;
	std::string bitrateByLayer;
};

/**
 * Producer type.
 */
// type ProducerType = 'simple' | 'simulcast' | 'svc';

//const logger = new Logger('Producer');

class Producer   : public EnhancedEventEmitter, public std::enable_shared_from_this<Producer>
{
	// Internal data.
public:
	struct internal
	{
		std::string routerId;
		std::string transportId;
		std::string producerId;
	};
  json _internal;
	// Producer data.
	struct data
	{
		std::string  kind;
		RtpParameters rtpParameters ;
		std::string type; // ProducerType;
		RtpParameters consumableRtpParameters;
	};
    json _data;
	// Channel instance.
	std::shared_ptr<ChannelAgent> _channel;

	// PayloadChannel instance.
	std::shared_ptr<PayloadChannelAgent> _payloadChannel;

	// Closed flag.
	bool _closed = false;

	// Custom app data.
	json _appData;

	// Paused flag.
	bool _paused = false;

	// Current score.
	std::vector<ProducerScore> _score;

	// Observer instance.
    EnhancedEventEmitter * _observer = new EnhancedEventEmitter();

	/**
	 * @private
	 * @emits transportclose
	 * @emits score - (score: ProducerScore[])
	 * @emits videoorientationchange - (videoOrientation: ProducerVideoOrientation)
	 * @emits trace - (trace: ProducerTraceEventData)
	 * @emits @close
	 */
	Producer(
    	json& internal,
			json& data,
			std::shared_ptr<ChannelAgent> &channel,
			std::shared_ptr<PayloadChannelAgent> &payloadChannel,
			json& appData,
             bool paused
	)
	{
		//super();

		MS_lOGD("Producer() data=%s",data.dump(4).c_str());

		this->_internal = internal;
		this->_data = data;
		this->_channel = channel;
		this->_payloadChannel = payloadChannel;
		this->_appData = appData;
		this->_paused = paused;

		//this->_handleWorkerNotifications();
	}

	/**
	 * Producer id.
	 */
	std::string id()
	{
		return this->_internal["producerId"];
	}

	/**
	 * Whether the Producer is closed.
	 */
	bool closed()
	{
		return this->_closed;
	}

	/**
	 * Media kind.
	 */
	std::string kind()
	{
        return this->_data["kind"];
	}

	/**
	 * RTP parameters.
	 */
   
	RtpParameters rtpParameters() 
	{
        //RtpParameters rtp;
        //from_json(this->_data["rtpParameters"],rtp);
        //return this->_data["rtpParameters"].get<RtpParameters>();
        return this->_data["rtpParameters"];
	}

	/**
	 * Producer type.
	 */
	std::string type() //ProducerType
	{
        return this->_data["type"];
	}

	/**
	 * Consumable RTP parameters.
	 *
	 * @private
	 */
	RtpParameters consumableRtpParameters()
	{
	    MS_lOGD("consumableRtpParameters=%s",this->_data["consumableRtpParameters"].dump(4).c_str());
        return this->_data["consumableRtpParameters"];
       
	}

	/**
	 * Whether the Producer is paused.
	 */
	bool paused()
	{
		return this->_paused;
	}

	/**
	 * Producer score list.
	 */
    std::vector<ProducerScore>  score()
	{
		return this->_score;
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
	 * @emits pause
	 * @emits resume
	 * @emits score - (score: ProducerScore[])
	 * @emits videoorientationchange - (videoOrientation: ProducerVideoOrientation)
	 * @emits trace - (trace: ProducerTraceEventData)
	 */
	EnhancedEventEmitter* observer()
	{
		return this->_observer;
	}
	/**
	 * Close the Producer.
	 */
	void close()
	{
		if (this->_closed)
			return;

		MS_lOGD("close()");

		this->_closed = true;

		// Remove notification subscriptions.
        this->_channel->removeAllListeners(this->_internal["producerId"]);
		this->_payloadChannel->removeAllListeners(this->_internal["producerId"]);

		this->_channel->request("producer.close", this->_internal);

		this->emit("@close");

		// Emit observer event.
		this->_observer->safeEmit("close", this->_internal["producerId"]);
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
		this->_channel->removeAllListeners(this->_internal["producerId"]);
		this->_payloadChannel->removeAllListeners(this->_internal["producerId"]);

		this->safeEmit("transportclose");

		// Emit observer event.
		this->_observer->safeEmit("close");
	}

	/**
	 * Dump Producer.
	 */
	json dump() //Promise<any>
	{
		MS_lOGD("dump()");

		return this->_channel->request("producer.dump",this->_internal);
	}

	/**
	 * Producer stats.
	 */
	json getStats() //Promise<ProducerStat[]>
	{
		MS_lOGD("getStats()");

		return this->_channel->request("producer.getStats",this->_internal);
	}

	/**
	 * Pause the Producer.
	 */
	void pause() //Promise<void>
	{
		MS_lOGD("pause()");

		const bool wasPaused = this->_paused;

		this->_channel->request("producer.pause", this->_internal);

		this->_paused = true;

		// Emit observer event.
		if (!wasPaused)
			this->_observer->safeEmit("pause");
	}

	/**
	 * Resume the Producer.
	 */
	void resume() //Promise<void>
	{
		MS_lOGD("resume()");

		const bool wasPaused = this->_paused;

		this->_channel->request("producer.resume", this->_internal);

		this->_paused = false;

		// Emit observer event.
		if (wasPaused)
			this->_observer->safeEmit("resume");
	}

	/**
	 * Enable 'trace' event.
	 */
	void enableTraceEvent(std::vector<std::string> types) // Promise<void>
	{
		MS_lOGD("enableTraceEvent()");

        const json reqData = {
            { "types" , types }
            
        };

        this->_channel->request(
			"producer.enableTraceEvent", this->_internal, reqData);

	}

	/**
	 * Send RTP packet (just valid for Producers created on a DirectTransport).
	 */
	void send(const char * rtpPacket)
	{
		//if (!Buffer.isBuffer(rtpPacket))
		//{
		//	MS_THROW_lOG("rtpPacket must be a Buffer");
		//}

		//this->_payloadChannel->notify(
		//	"producer.send", this->_internal, "undefined", rtpPacket);
	}
  
  void processChannelNotifications(std::string event,const json & data) 
  {
    //	switch (event)
		//	{
				if(event == "score")
				{
                    std::vector<ProducerScore> score = data;
          //auto score = data;
					this->_score = score;
                
					this->safeEmit("score", score);

					// Emit observer event.
                    
					this->_observer->safeEmit("score", score);

					//break;
				}else
				if(event == "videoorientationchange")
				{
                   
                    auto  videoOrientation = data;
                  
					this->safeEmit("videoorientationchange", videoOrientation);

					// Emit observer event.
					this->_observer->safeEmit("videoorientationchange", videoOrientation);

					//break;
				} else
				if(event == "trace")
				{
                    auto trace = data;

					this->safeEmit("trace", trace);

					// Emit observer event.
					this->_observer->safeEmit("trace", trace);

					//break;
				} else 
				{
					MS_lOGE("ignoring unknown event %s", event.c_str());
				}
		//	}
  }
  void processPayloadChannelNotifications(std::string event,const json & data) 
  {
  }
	void handleWorkerNotifications()
	{
		this->_channel->on(this->_internal["producerId"].get<std::string>(),[self = shared_from_this()]( std::string event,json data ) //this->_internal["producerId"], (event, data?: any) =>
		{		  
			self->processChannelNotifications(event,data);
		});
	}
};


}
