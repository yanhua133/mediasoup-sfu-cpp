#include "Mediasoup.hpp"
#include "Log.hpp"
#include "Promise.hpp"
#include "WorkerAgent.hpp"
#include "SupportedRtpCapabilities.hpp"



void * g_uvloop = NULL;
namespace mediasoup
{

mediasoup::IMediasoup* CreateMediasoup() {
    MS_lOGF();
    return &mediasoup::Mediasoup::GetInstance();
}

void DestroyMediasoup(mediasoup::IMediasoup* mediasoup) {
    MS_lOGF();
    if (!mediasoup) {
        mediasoup->Destroy();
        return;
    }
}

Mediasoup::Mediasoup() {
    MS_lOGF();
}

Mediasoup::~Mediasoup() {
    MS_lOGF();
}
promise::Defer writeTo(std::ostream& out) {
    return promise::newPromise().then([&out](int value) {
        out << value;
        MS_lOGD("writeTo 1=%d",value);
        return promise::reject(std::string(" testErrorReason "));
    }, [&out](const std::string& reason) {
        out << reason;
        MS_lOGD("writeTo 2 =%s",reason.c_str());
        return 456;
    });
}
void Mediasoup::Test() {
    return;
  
}

void StaticWorkerFun(void* arg) {
    MS_lOGF();
    static_cast<Mediasoup*>(arg)->WorkerFun();
}

void StaticAsync(uv_async_t* handle) {
    MS_lOGF();
    static_cast<Mediasoup*>(handle->loop->data)->Async(handle);
}

bool Mediasoup::Init() {
    MS_lOGF();
	MS_ASSERT_RV_LOGI(!m_loop, true, "already Init");

#if 0
	m_loop = new uv_loop_t;
	MS_ASSERT_RV_LOGE(m_loop, false, "create loop failed");

	uv_loop_init(m_loop);

	// save this
	m_loop->data = static_cast<void*>(this);

	uv_async_init(m_loop, &m_async, StaticAsync);

	// run loop
    int ret = uv_thread_create(&m_workThread, StaticWorkerFun, this);
    if (0 != ret) {
        MS_lOGE("uv_thread_create failed:{}", uv_strerror(ret));
		Destroy();
        return false;
    }
#endif
	return true;
}

void Mediasoup::Destroy()  {
    MS_lOGF();
#if 0
	MS_ASSERT_R_LOGI(m_loop, "need Init first");

    // notify quit
    uv_async_send(&m_async);

    MS_lOGI("wait work thread quit");
    uv_thread_join(&m_workThread); 

	// clear works
	for (const auto& it : m_works) {
		if (!it) {
			continue;
		}
		//delete it;
	}

	uv_loop_close(m_loop);
	delete m_loop;
	m_loop = nullptr;
#endif
    MS_lOGI("work thread quit");
}

std::shared_ptr<mediasoup::IWorker> Mediasoup::CreateWorker(IWorker::Observer* workerObserver, const WorkerSettings& workerSettings)  {
    MS_lOGF();
    std::shared_ptr<mediasoup::WorkerAgent> work = std::make_shared<WorkerAgent>(workerObserver, workerSettings);
	if (!work->Init(workerSettings.producerFd,workerSettings.consumerFd,workerSettings.payloadProducerFd,workerSettings.payloadConsumerFd)) {
		MS_lOGE("worker init failed");
		//delete work;
		return nullptr;
	}

	m_works.push_back(work);
    return work;
}


RtpCapabilities Mediasoup::GetSupportedRtpCapabilities() {
	return supportedRtpCapabilities;
}

uv_loop_t* Mediasoup::GetLoop()
{
    return (uv_loop_t*)g_uvloop;
   // return DepLibUV::GetLoop();
	//return m_loop;
}

void Mediasoup::WorkerFun() {    
    MS_lOGI("WorkerFun begine");
#if 0
    MS_lOGI("uv_run");
    uv_run(m_loop, UV_RUN_DEFAULT);    
#endif
    MS_lOGI("WorkerFun end");
}

void Mediasoup::Async(uv_async_t* handle) {
    MS_lOGF();
    uv_stop(handle->loop);
}

}
