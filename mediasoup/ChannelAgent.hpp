#pragma once
#include <memory>
#include <string>
#include <string>
#include "uv/UVPipeWrapper.hpp"
#include "common/CommonObserver.hpp"
#include "json.hpp"
#include "EnhancedEventEmitter.hpp"
#include "Promise.hpp"
#include "util/timer.hpp"
#include <stdexcept>
#include <future>
#include <chrono>
using json = nlohmann::json;
namespace mediasoup
{
struct Sent
{
	int id;
	std::string method;
	std::function<void(json)> resolve;
	std::function<void(std::string)> reject;
	std::function<void()> close;
};
class PeerError : public std::runtime_error
	{
	public:
		explicit PeerError(const char* description);
	};

	/* Inline methods. */

	inline PeerError::PeerError(const char* description)
		: std::runtime_error(description)
	{
	}
class ChannelObserver {
public:
	virtual void OnRunning() = 0;
	virtual void OnMsg(std::string targetId, std::string event, std::string data) = 0;
};

class ChannelAgent : public EnhancedEventEmitter,public CommonObserver<ChannelObserver>, public UVPipeObserver {
public:
    ChannelAgent(ChannelObserver* obs);
    virtual ~ChannelAgent();

	bool Init(int producerFd,int consumerFd);
	bool Start();
	void SetPid(int pid);
	UVPipeWrapper* GetProducer();
	UVPipeWrapper* GetConsumer();

	// UVPipeObserver
	void OnRead(UVPipeWrapper* pipe, uint8_t* data, size_t len) override;
	void OnClose(UVPipeWrapper* pipe) override;
	void Write(const uint8_t* data, size_t len);
    void removeAllListeners(std::string producerId);
    json request(const char * method, json& internal);
    json request(const char * method, json& internal,const json& data);

    json notify(std::string method,json& internal,json& notifData,std::string message);
protected:
	void processMessage(const nlohmann::json& jsonMessage);

private:
	int m_pid = -1;
	std::mutex m_mutex;
	std::unique_ptr<UVPipeWrapper> m_producerPipe = nullptr;
	std::unique_ptr<UVPipeWrapper> m_consumerPipe = nullptr;

    std::string m_readBuf;
    	// Next id for messages sent to the worker process.
	  size_t _nextId = 0;
    // Closed flag.
	  bool _closed = false;
	  // Map of pending sent requests.
	  // _sents: Map<number, Sent> = new Map();
    std::map<size_t,std::shared_ptr<Sent>> m_sents;
	  // Buffer for reading messages from the worker.
    //private _recvBuffer?: Buffer;
	  bool m_once = false;
    ChannelTimer mTimer;
    std::unique_ptr<std::promise<json>> m_pPromise;
};

}
