#pragma once
#include "Log.hpp"
#include "EnhancedEventEmitter.hpp"
#include "RtpObserver.hpp"
#include "Producer.hpp"

#include "Promise.hpp"
using json = nlohmann::json;
using namespace mediasoup;

namespace mediasoup
{
  
struct AudioLevelObserverOptions
{
	/**
	 * Maximum number of entries in the 'volumes”' event. Default 1.
	 */
	int maxEntries;

	/**
	 * Minimum average volume (in dBvo from -127 to 0) for entries in the
	 * 'volumes' event.	Default -80.
	 */
	int threshold;

	/**
	 * Interval in ms for checking audio volumes. Default 1000.
	 */
	int interval;

	/**
	 * Custom application data.
	 */
	json appData;
};

struct AudioLevelObserverVolume
{
	/**
	 * The audio producer instance.
	 */
    std::shared_ptr<Producer> producer;

	/**
	 * The average volume (in dBvo from -127 to 0) of the audio producer in the
	 * last interval.
	 */
	int volume;
};

//onst logger = new Logger('AudioLevelObserver');

class AudioLevelObserver : public RtpObserver, public std::enable_shared_from_this<AudioLevelObserver>
{
public:
	/**
	 * @private
	 * @emits volumes - (volumes: AudioLevelObserverVolume[])
	 * @emits silence
	 */
	AudioLevelObserver(RtpObserverParams &params):RtpObserver(params)
	{
		//super(params);

		//this->_handleWorkerNotifications();
	}

	/**
	 * Observer.
	 *
	 * @emits close
	 * @emits pause
	 * @emits resume
	 * @emits addproducer - (producer: Producer)
	 * @emits removeproducer - (producer: Producer)
	 * @emits volumes - (volumes: AudioLevelObserverVolume[])
	 * @emits silence
	 */
	EnhancedEventEmitter * observer()
	{
		return this->_observer;
	}

	void handleWorkerNotifications()
	{
		this->_channel->on(this->_internal["rtpObserverId"].get<std::string>(), [self = shared_from_this()]( std::string event,json data ) //(event: string, data?: any) =>
		{
			if (true)
			{
				if(event == "volumes")
				{
					// Get the corresponding Producer instance and remove entries with
					// no Producer (it may have been closed in the meanwhile).
//					const volumes: AudioLevelObserverVolume[] = data
//						.map(({ producerId, volume }: { producerId: string; volume: number }) => (
//							{
//								producer : this->_getProducerById(producerId),
//								volume
//							}
//						))
//						.filter(({ producer }: { producer: Producer }) => producer);

                    std::vector<AudioLevelObserverVolume> volumes;// = new std::vector<AudioLevelObserverVolume>();
                    for ( auto &it : data)
                    {
                        AudioLevelObserverVolume vol;
                        std::string producerId = it["producerId"];
                        vol.producer = self->_getProducerById(producerId);
                        vol.volume = it["volume"].get<int>();
                        volumes.push_back(vol);
                        
                    }
					if (volumes.size() > 0)
					{
						self->safeEmit("volumes", &volumes);

						// Emit observer event.
						self->_observer->safeEmit("volumes", &volumes);
					}

					//break;
				}else
                if(event == "silence")
				{
					self->safeEmit("silence");

					// Emit observer event.
					self->_observer->safeEmit("silence");

					//break;
				}else
				{
                    MS_lOGE("ignoring unknown event %s", event.c_str());
				}
			}
		});
	}
};

}
