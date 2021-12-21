#pragma once
#include "IWorker.hpp"
#include "common/CommonObserver.hpp"
#include "uv.h"
#include "uv/UVPipeWrapper.hpp"
#include "ChannelAgent.hpp"
#include "PayloadChannelAgent.hpp"
#include "Router.hpp"
namespace mediasoup
{
class WorkerAgent : public IWorker
	, public CommonObserver<IWorker::Observer>
	, public UVPipeObserver 
	, public ChannelObserver
	, public std::enable_shared_from_this<WorkerAgent>{
public:
    WorkerAgent(IWorker::Observer* obs, const WorkerSettings& workerSettings);
    virtual ~WorkerAgent();

	bool Init(int producerFd,int consumerFd,int payloadProducerFd,int payloadConsumerFd);
	void Close();

	void OnExit(int64_t exitStatus, int termSignal);
	// UVPipeObserver
	void OnRead(UVPipeWrapper* pipe, uint8_t* data, size_t len) override;
	void OnClose(UVPipeWrapper* pipe) override;

	// ChannelObserver
	void OnRunning() override;
	void OnMsg(std::string targetId, std::string event, std::string data) override;
	void Write(const uint8_t* data, size_t len) override;
  void PayloadWrite(const uint8_t* data, size_t len) override;
  void Write(std::string data) override;
  void PayloadWrite(std::string data) override;
  void updateSettings(WorkerUpdateableSettings &settings);
  std::shared_ptr<Router>  createRouter(RouterOptions & options);
protected:
	void getWorkBinPath();
	
private:
	static std::string s_workerBin;
	WorkerSettings m_settings;
	uv_process_t* m_child = nullptr;

	std::unique_ptr<UVPipeWrapper> m_childStdOut = nullptr;
	std::unique_ptr<UVPipeWrapper> m_childStdErr = nullptr;
	std::shared_ptr<ChannelAgent> m_channel = nullptr;
  std::shared_ptr<PayloadChannelAgent> m_payloadChannel = nullptr;
  std::vector<std::shared_ptr<Router>> m_routers;

	int m_pid = -1;
	bool m_spawnDone = false;
};

}
