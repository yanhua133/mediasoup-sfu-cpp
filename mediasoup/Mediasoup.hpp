#pragma once
#include <vector>

#include "IMediasoup.hpp"
#include "Singleton.hpp"
#include "uv.h"
#include "pod_circularbuffer.hpp"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE (1024*100)
#endif

#ifndef MAXLISTSIZE
#define MAXLISTSIZE 500
#endif

namespace mediasoup
{

typedef struct _write_param {//param of uv_write
    uv_write_t write_req_;//store TCPClient on data
    uv_buf_t buf_;
    int buf_truelen_;
    uv_pipe_t* pipHandle_;
}write_param;

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

public:
    bool m_isUserAskforclosed; 
    void send_inl(uv_write_t* req = NULL);
    int Send(const char* data, std::size_t len, uv_pipe_t* pipHandle);


    write_param* AllocWriteParam(int bufLen = BUFFER_SIZE, uv_pipe_t* pipHandle = nullptr);
    void FreeWriteParam(write_param* param);
    std::string errmsg_;

    //send param
    uv_mutex_t mutex_writebuf_;//mutex of writebuf_list_
    std::list<write_param*> writeparam_list_;//Availa write_t
    PodCircularBuffer<char> write_circularbuf_;//the data prepare to send
};

}
