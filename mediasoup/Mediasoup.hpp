#pragma once
#include <vector>

#include "IMediasoup.hpp"
#include "Singleton.hpp"
#include "uv.h"

namespace mediasoup
{

class Mediasoup : public IMediasoup, public mediasoup::Singleton<Mediasoup> {
public:
    Mediasoup();
    virtual ~Mediasoup();

    virtual void Test() override;
	virtual bool Init() override;
    virtual void Destroy() override;
    virtual std::shared_ptr<mediasoup::IWorker> CreateWorker(IWorker::Observer* workerObserver, const WorkerSettings& workerSettings) override;
	virtual RtpCapabilities GetSupportedRtpCapabilities() override;

	uv_loop_t* GetLoop();

public:
    void WorkerFun();
    void Async(uv_async_t* handle);

private:
    uv_thread_t m_workThread;    
	uv_loop_t* m_loop = nullptr;
    uv_async_t m_async;
	
	std::vector<std::shared_ptr<mediasoup::IWorker>> m_works;
    
};

}
