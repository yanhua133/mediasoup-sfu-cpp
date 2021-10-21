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

Mediasoup::Mediasoup() 
    :write_circularbuf_(BUFFER_SIZE), m_isUserAskforclosed(false)
{
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
    Mediasoup* theclass = (Mediasoup*)handle->loop->data;

    theclass->Async(handle);
    //static_cast<Mediasoup*>(handle->loop->data)->Async(handle);
}

bool Mediasoup::Init() {
    MS_lOGF();
	MS_ASSERT_RV_LOGI(!m_loop, true, "already Init");

    int iret = uv_mutex_init(&mutex_writebuf_);
    if (iret) {
        return false;
    }

#if 1 
	m_loop = new uv_loop_t;
	MS_ASSERT_RV_LOGE(m_loop, false, "create loop failed");

	uv_loop_init(m_loop);

	// save this
	m_loop->data = static_cast<void*>(this);

	uv_async_init(m_loop, &m_async, StaticAsync);

	// run loop
    int ret = uv_thread_create(&m_workThread, StaticWorkerFun, this);
    if (0 != ret) {
        MS_lOGE("uv_thread_create failed:{%s}", uv_strerror(ret));
		Destroy();
        return false;
    }
#endif
	return true;
}

void Mediasoup::Destroy()  {
    MS_lOGF();
#if 1 
	MS_ASSERT_R_LOGI(m_loop, "need Init first");

    m_isUserAskforclosed = true;
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

    uv_mutex_destroy(&mutex_writebuf_);
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
#if 1 
    return m_loop;
#else
    return (uv_loop_t*)g_uvloop;
#endif
   // return DepLibUV::GetLoop();
	//return m_loop;
}

void Mediasoup::WorkerFun() {    
    MS_lOGI("WorkerFun begine");
#if 1 
    MS_lOGI("uv_run");
    uv_run(m_loop, UV_RUN_DEFAULT);    
#endif
    MS_lOGI("WorkerFun end");
}
//uv_async_send 触发的异步回调函数
void Mediasoup::Async(uv_async_t* handle) {
    MS_lOGF();
    Mediasoup* theclass = (Mediasoup*)handle->loop->data;

    if (m_isUserAskforclosed == true) {
        uv_stop(handle->loop);
        return;
    }
    //处理需要发送的数据。因为libuv函数是非线程安全的，所以，需要在loop所在函数，调用uv_write 函数，发送数据
    //check data to send
    theclass->send_inl(NULL);
}

void AfterSend(uv_write_t* req, int status)
{
    Mediasoup* theclass = (Mediasoup*)req->data;

    theclass->FreeWriteParam((write_param*)req); 


}

int Mediasoup::Send(const char* data, std::size_t len, uv_pipe_t* pipHandle)
{
    if (!data || len <= 0) {
        errmsg_ = "send data is null or len less than zero.";
        MS_lOGE(errmsg_.c_str());
        return 0;
    }
    uv_async_send(&m_async);
    size_t iret = 0;
    write_param* writep = nullptr;

    if (!m_isUserAskforclosed) {

        writep = AllocWriteParam(len, pipHandle);
        writep->write_req_.data = this;
        memcpy(writep->buf_.base, data, len);

        uv_mutex_lock(&mutex_writebuf_);
        if (writeparam_list_.size() > MAXLISTSIZE) {
            FreeWriteParam(writep);
        }
        else {
            writeparam_list_.push_back(writep);
        }
        uv_mutex_unlock(&mutex_writebuf_);
    }
    uv_async_send(&m_async);
    return iret;
}

void Mediasoup::send_inl(uv_write_t* req /*= NULL*/)
{
    write_param* writep = (write_param*)req;
    if (writep) {
        if (writeparam_list_.size() > MAXLISTSIZE) {
            FreeWriteParam(writep);
        }
        else {
            writeparam_list_.push_back(writep);
        }
    }
    if (!m_isUserAskforclosed) {
        uv_mutex_lock(&mutex_writebuf_);

        if (!writeparam_list_.empty()) {
            writep = writeparam_list_.front();
            writeparam_list_.pop_front();
        }
        uv_mutex_unlock(&mutex_writebuf_);

        if (writep == nullptr)
            return;

        int iret = uv_write((uv_write_t*)&writep->write_req_, (uv_stream_t*)writep->pipHandle_, &writep->buf_, 1, AfterSend);
        if (iret) {
            writeparam_list_.push_back(writep);//failure not call AfterSend. so recycle req
            MS_lOGI("client  send error: %s", uv_strerror(iret));
            fprintf(stdout, "send error. %s-%s\n", uv_err_name(iret), uv_strerror(iret));
        }
    }
}


write_param* Mediasoup::AllocWriteParam(int bufLen , uv_pipe_t* pipHandle)
{
    write_param* param = (write_param*)malloc(sizeof(write_param));
    if ((bufLen <= 0)||(param ==nullptr))
        return nullptr;
    param->buf_.base = (char*)malloc(bufLen);
    param->buf_.len = bufLen;
    param->buf_truelen_ = bufLen;
    param->pipHandle_ = pipHandle;
    return param;
}

void Mediasoup::FreeWriteParam(write_param* param)
{
    free(param->buf_.base);
    free(param);
}

}
