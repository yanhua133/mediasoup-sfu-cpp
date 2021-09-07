#pragma once
#include "IWorker.hpp"
#include "RtpParameters.hpp"
#include "Version.hpp"

namespace mediasoup
{

class IMediasoup {
    public:
    virtual ~IMediasoup() {}

    virtual void Test() = 0;
	  virtual bool Init() = 0;
    virtual void Destroy() = 0;

	  /**
	   * Create a Worker.
	   */
    virtual std::shared_ptr<mediasoup::IWorker> CreateWorker(IWorker::Observer* workerObserver, const WorkerSettings& workerSettings) = 0;

	  /**
	   * Get a cloned copy of the mediasoup supported RTP capabilities.
	   */
	  virtual RtpCapabilities GetSupportedRtpCapabilities() = 0;
};

mediasoup::IMediasoup* CreateMediasoup();

void DestroyMediasoup(mediasoup::IMediasoup* mediasoup);

}
