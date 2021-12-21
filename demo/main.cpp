#include <stdio.h>
#include <iostream>
#include "Config.hpp"

#include "common.hpp"
#include "DepLibSRTP.hpp"
#include "DepLibUV.hpp"
#include "DepLibWebRTC.hpp"
#include "DepOpenSSL.hpp"
#include "DepUsrSCTP.hpp"
#include "Logger.hpp"
#include "MediaSoupErrors.hpp"
#include "Settings.hpp"
#include "Utils.hpp"
#include "Worker.hpp"
#include "Channel/Notifier.hpp"
#include "Channel/UnixStreamSocket.hpp"
#include "PayloadChannel/Notifier.hpp"
#include "PayloadChannel/UnixStreamSocket.hpp"
#include "RTC/DtlsTransport.hpp"
#include "RTC/SrtpSession.hpp"
#include <uv.h>
#include <cerrno>
#include <csignal>  // sigaction()
#include <cstdlib>  // std::_Exit(), std::genenv()
#include <iostream> // std::cerr, std::endl
#include <map>
#include <string>
#include "IMediasoup.hpp"
#include "IWorker.hpp"
#include "Log.hpp"
#include "Consumer.hpp"

#include "controller/StatisticsController.hpp"
#include "controller/RoomsController.hpp"
#include "./AppComponent.hpp"
#include "oatpp/network/Server.hpp"

#define MS_CLASS "mediasoup-worker"
// #define MS_LOG_DEV_LEVEL 3
#include "Server.hpp"
#ifndef _WIN32
# include <unistd.h>  /* close */

#else
#include <fcntl.h>
#include <io.h>
#endif
static int ConsumerChannelFd[2]={3,4};
static int ProducerChannelFd[2]={5,6};
static int PayloadConsumerChannelFd[2]={7,8};
static int PayloadProducerChannelFd[2]={9,10};
extern void * g_uvloop;
void IgnoreSignals();
// Channel socket (it will be handled and deleted by the Worker).
Channel::UnixStreamSocket* channel{ nullptr };

// PayloadChannel socket (it will be handled and deleted by the Worker).
PayloadChannel::UnixStreamSocket* payloadChannel{ nullptr };

int worker_init(int argc, const char* argv[])
{

    std::string version = "m1";//std::getenv("MEDIASOUP_VERSION");

	// Initialize libuv stuff (we need it for the Channel).
	//DepLibUV::ClassInit();

    MS_lOGD("raw Worker ConsumerChannelFd[0]=%d,ProducerChannelFd[1]=%d,PayloadConsumerChannelFd[0]=%d,PayloadProducerChannelFd[1]=%d",ConsumerChannelFd[0],ProducerChannelFd[1],PayloadConsumerChannelFd[0],PayloadProducerChannelFd[1]);

	try
	{
		channel = new Channel::UnixStreamSocket(ConsumerChannelFd[0], ProducerChannelFd[1]);
	}
	catch (const MediaSoupError& error)
	{
		MS_ERROR_STD("error creating the Channel: %s", error.what());

		std::_Exit(EXIT_FAILURE);
	}

	try
	{
		payloadChannel =
		  new PayloadChannel::UnixStreamSocket(PayloadConsumerChannelFd[0], PayloadProducerChannelFd[1]);
	}
	catch (const MediaSoupError& error)
	{
		MS_ERROR_STD("error creating the RTC Channel: %s", error.what());

		std::_Exit(EXIT_FAILURE);
	}

	// Initialize the Logger.
	Logger::ClassInit(channel);

	try
	{
		Settings::SetConfiguration(argc, argv);
	}
	catch (const MediaSoupTypeError& error)
	{
		MS_ERROR_STD("settings error: %s", error.what());

		// 42 is a custom exit code to notify "settings error" to the Node library.
		std::_Exit(42);
	}
	catch (const MediaSoupError& error)
	{
		MS_ERROR_STD("unexpected settings error: %s", error.what());

		std::_Exit(EXIT_FAILURE);
	}

	MS_DEBUG_TAG(info, "starting mediasoup-worker process [version:%s]", version.c_str());

#if defined(MS_LITTLE_ENDIAN)
	MS_DEBUG_TAG(info, "little-endian CPU detected");
#elif defined(MS_BIG_ENDIAN)
	MS_DEBUG_TAG(info, "big-endian CPU detected");
#else
	MS_WARN_TAG(info, "cannot determine whether little-endian or big-endian");
#endif

#if defined(INTPTR_MAX) && defined(INT32_MAX) && (INTPTR_MAX == INT32_MAX)
	MS_DEBUG_TAG(info, "32 bits architecture detected");
#elif defined(INTPTR_MAX) && defined(INT64_MAX) && (INTPTR_MAX == INT64_MAX)
	MS_DEBUG_TAG(info, "64 bits architecture detected");
#else
	MS_WARN_TAG(info, "cannot determine 32 or 64 bits architecture");
#endif

	Settings::PrintConfiguration();
	DepLibUV::PrintVersion();

	try
	{
		// Initialize static stuff.
		DepOpenSSL::ClassInit();
		DepLibSRTP::ClassInit();
		DepUsrSCTP::ClassInit();
		DepLibWebRTC::ClassInit();
		Utils::Crypto::ClassInit();
		RTC::DtlsTransport::ClassInit();
		RTC::SrtpSession::ClassInit();
		Channel::Notifier::ClassInit(channel);
		PayloadChannel::Notifier::ClassInit(payloadChannel);

		// Ignore some signals.
		IgnoreSignals();

		// Run the Worker.
		Worker worker(channel, payloadChannel);

		// Free static stuff.
		DepLibUV::ClassDestroy();
		DepLibSRTP::ClassDestroy();
		Utils::Crypto::ClassDestroy();
		DepLibWebRTC::ClassDestroy();
		RTC::DtlsTransport::ClassDestroy();
		DepUsrSCTP::ClassDestroy();

		// Wait a bit so peding messages to stdout/Channel arrive to the Node
		// process.
		uv_sleep(200);

		//std::_Exit(EXIT_SUCCESS);
	}
	catch (const MediaSoupError& error)
	{
		MS_ERROR_STD("failure exit: %s", error.what());

		//std::_Exit(EXIT_FAILURE);
	}
  
}

int uv_pipe_test()
{
  uv_pipe_t pipe_handle;
  uv_pipe_t pipe_handle2;
  int fd[2]={0,1};
 
  int  ret = 0;
#ifndef WIN32
   pipe(fd);
#endif
   uv_pipe_init(uv_default_loop(), &pipe_handle, 0);
   uv_pipe_open(&pipe_handle, fd[0]);
    
   uv_pipe_init(uv_default_loop(), &pipe_handle2, 0);
   uv_pipe_open(&pipe_handle2, fd[1]);

}

#ifndef _WIN32
static int mpipe(int *fds) {
  if (pipe(fds) == -1)
    return -1;
  if (fcntl(fds[0], F_SETFD, FD_CLOEXEC) == -1 ||
      fcntl(fds[1], F_SETFD, FD_CLOEXEC) == -1) {
    close(fds[0]);
    close(fds[1]);
    return -1;
  }
  return 0;
}
#else
static int mpipe(int *fds) {
  SECURITY_ATTRIBUTES attr;
  HANDLE readh, writeh;
  attr.nLength = sizeof(attr);
  attr.lpSecurityDescriptor = NULL;
  attr.bInheritHandle = FALSE;
  if (!CreatePipe(&readh, &writeh, &attr, 0))
    return -1;
  fds[0] = _open_osfhandle((intptr_t)readh, 0);
  fds[1] = _open_osfhandle((intptr_t)writeh, 0);
  if (fds[0] == -1 || fds[1] == -1) {
    CloseHandle(readh);
    CloseHandle(writeh);
    return -1;
  }
  return 0;
}
#endif

void run(int argc, const char* argv[]) {
    
  /* Register Components in scope of run() method */
  AppComponent components(oatpp::base::CommandLineArguments(argc, argv));

  /* Get router component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

  /* Create RoomsController and add all of its endpoints to router */
  auto roomsController = std::make_shared<RoomsController>();
  roomsController->addEndpointsToRouter(router);

  auto statisticsController = std::make_shared<StatisticsController>();
  statisticsController->addEndpointsToRouter(router);

  /* Get connection handler component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler, "http");

  /* Get connection provider component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

  /* Create server which takes provided TCP connections and passes them to HTTP connection handler */
  oatpp::network::Server server(connectionProvider, connectionHandler);

  std::thread serverThread([&server]{
      std::ostringstream ss;
      ss << std::this_thread::get_id();
      MS_lOGD("[Room] serverThread thread:%s",ss.str().c_str());
    server.run();
  });
    //std::shared_ptr<SfuServer> sfuServer = std::make_shared<SfuServer>();
  OATPP_COMPONENT(std::shared_ptr<SfuServer>, sfuServer);
  std::thread pingThread([]{
    OATPP_COMPONENT(std::shared_ptr<SfuServer>, sfuServer);
    //sfuServer->runPingLoop(std::chrono::seconds(30));
  });

//  std::thread statThread([]{
//    OATPP_COMPONENT(std::shared_ptr<Statistics>, statistics);
//    statistics->runStatLoop();
//  });

  OATPP_COMPONENT(oatpp::Object<ConfigDto>, appConfig);

  if(appConfig->useTLS) {
    OATPP_LOGI("main.cpp", "clients are expected to connect at https://%s:%d/", appConfig->host->c_str(), *appConfig->port);
  } else {
    OATPP_LOGI("main.cpp", "clients are expected to connect at http://%s:%d/", appConfig->host->c_str(), *appConfig->port);
  }

  OATPP_LOGI("main.cpp", "canonical base URL=%s", appConfig->getCanonicalBaseUrl()->c_str());
  OATPP_LOGI("main.cpp", "statistics URL=%s", appConfig->getStatsUrl()->c_str());
  
    //SfuServer server;
    auto pConfig = std::make_shared<Config>();
    pConfig->initConfig();
    DepLibUV::ClassInit();
    g_uvloop =  DepLibUV::GetLoop();
    mpipe(ConsumerChannelFd);
    MS_lOGD("pipe Create Pair ConsumerChannelFd[0]=%d ConsumerChannelFd[1]=%d ",ConsumerChannelFd[0],ConsumerChannelFd[1]);
    mpipe(ProducerChannelFd);
    MS_lOGD("pipe Create Pair ProducerChannelFd[0]=%d ProducerChannelFd[1]=%d ",ProducerChannelFd[0],ProducerChannelFd[1]);
    mpipe(PayloadConsumerChannelFd);
    MS_lOGD("pipe Create Pair PayloadConsumerChannelFd[0]=%d PayloadConsumerChannelFd[1]=%d ",PayloadConsumerChannelFd[0],PayloadConsumerChannelFd[1]);
    mpipe(PayloadProducerChannelFd);
    MS_lOGD("pipe Create Pair PayloadProducerChannelFd[0]=%d PayloadProducerChannelFd[1]=%d ",PayloadProducerChannelFd[0],PayloadProducerChannelFd[1]);
  
    sfuServer->setConfig(pConfig);
    
    
    std::thread workerThread([&argc, &argv, &sfuServer]{
        std::ostringstream ss;
        ss << std::this_thread::get_id();
        sfuServer->initMediasoup();
        MS_lOGD("initWorker ProducerChannelFd[0]=%d,ConsumerChannelFd[1]=%d,PayloadProducerChannelFd[0]=%d,PayloadConsumerChannelFd[1]=%d",ProducerChannelFd[0],ConsumerChannelFd[1],PayloadProducerChannelFd[0],PayloadConsumerChannelFd[1]);
        sfuServer->initWorker(ProducerChannelFd[0],ConsumerChannelFd[1],PayloadProducerChannelFd[0],PayloadConsumerChannelFd[1]);
        worker_init(argc,argv);
        MS_lOGD("workerThread ended");
    });
    
    serverThread.join();
    pingThread.join();
    workerThread.join();
  //statThread.join();

}

int main(int argc, const char* argv[])
{
    oatpp::base::Environment::init();
    std::ostringstream ss;
    ss << std::this_thread::get_id();
    MS_lOGD("[Room] mainThread:%s",ss.str().c_str());
    run(argc, argv);
    oatpp::base::Environment::destroy();


    //worker_init(argc,argv);
   
   
    
    //getchar();

}

void IgnoreSignals()
{
#ifndef _WIN32
	MS_TRACE();

	int err;
	struct sigaction act; // NOLINT(cppcoreguidelines-pro-type-member-init)

	// clang-format off
	std::map<std::string, int> ignoredSignals =
	{
		{ "PIPE", SIGPIPE },
		{ "HUP",  SIGHUP  },
		{ "ALRM", SIGALRM },
		{ "USR1", SIGUSR1 },
		{ "USR2", SIGUSR2 }
	};
	// clang-format on

	act.sa_handler = SIG_IGN; // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
	act.sa_flags   = 0;
	err            = sigfillset(&act.sa_mask);

	if (err != 0)
		MS_THROW_ERROR("sigfillset() failed: %s", std::strerror(errno));

	for (auto& kv : ignoredSignals)
	{
		const auto& sigName = kv.first;
		int sigId           = kv.second;

		err = sigaction(sigId, &act, nullptr);

		if (err != 0)
			MS_THROW_ERROR("sigaction() failed for signal %s: %s", sigName.c_str(), std::strerror(errno));
	}
#endif
}
