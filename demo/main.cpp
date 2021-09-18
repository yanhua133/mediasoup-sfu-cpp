#include <stdio.h>
#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
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
#define MS_CLASS "mediasoup-worker"
// #define MS_LOG_DEV_LEVEL 3
#include "Server.hpp"

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

int worker_init(int argc, char* argv[])
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
int main(int argc, char* argv[])
{
    Server server;
    Config config;
    config.initConfig();
    g_uvloop =  DepLibUV::GetLoop();
#ifndef WIN32
    pipe(ConsumerChannelFd);
    MS_lOGD("pipe Create Pair ConsumerChannelFd[0]=%d ConsumerChannelFd[1]=%d ",ConsumerChannelFd[0],ConsumerChannelFd[1]);
    pipe(ProducerChannelFd);
    MS_lOGD("pipe Create Pair ProducerChannelFd[0]=%d ProducerChannelFd[1]=%d ",ProducerChannelFd[0],ProducerChannelFd[1]);
    pipe(PayloadConsumerChannelFd);
    MS_lOGD("pipe Create Pair PayloadConsumerChannelFd[0]=%d PayloadConsumerChannelFd[1]=%d ",PayloadConsumerChannelFd[0],PayloadConsumerChannelFd[1]);
    pipe(PayloadProducerChannelFd);
    MS_lOGD("pipe Create Pair PayloadProducerChannelFd[0]=%d PayloadProducerChannelFd[1]=%d ",PayloadProducerChannelFd[0],PayloadProducerChannelFd[1]);
#else
    uv_fs_t file_req;
    ConsumerChannelFd[0] = uv_fs_open((uv_loop_t*)g_uvloop, &file_req, "\\\\.\\Pipe\\ConsumerChannelFd0", O_CREAT | O_RDWR, 0644, NULL);
    ConsumerChannelFd[1] = uv_fs_open((uv_loop_t*)g_uvloop, &file_req, "\\\\.\\Pipe\\ConsumerChannelFd1", O_CREAT | O_RDWR, 0644, NULL);
    MS_lOGD("pipe Create Pair ConsumerChannelFd[0]=%d ConsumerChannelFd[1]=%d ",ConsumerChannelFd[0],ConsumerChannelFd[1]);
    ProducerChannelFd[0] = uv_fs_open((uv_loop_t*)g_uvloop, &file_req, "\\\\.\\Pipe\\ProducerChannelFd0", O_CREAT | O_RDWR, 0644, NULL);
    ProducerChannelFd[1] = uv_fs_open((uv_loop_t*)g_uvloop, &file_req, "\\\\.\\Pipe\\ProducerChannelFd1", O_CREAT | O_RDWR, 0644, NULL);
    MS_lOGD("pipe Create Pair ProducerChannelFd[0]=%d ProducerChannelFd[1]=%d ",ProducerChannelFd[0],ProducerChannelFd[1]);
    PayloadConsumerChannelFd[0] = uv_fs_open((uv_loop_t*)g_uvloop, &file_req, "\\\\.\\Pipe\\PayloadConsumerChannelFd0", O_CREAT | O_RDWR, 0644, NULL);
    PayloadConsumerChannelFd[1] = uv_fs_open((uv_loop_t*)g_uvloop, &file_req, "\\\\.\\Pipe\\PayloadConsumerChannelFd1", O_CREAT | O_RDWR, 0644, NULL);
    MS_lOGD("pipe Create Pair PayloadConsumerChannelFd[0]=%d PayloadConsumerChannelFd[1]=%d ",PayloadConsumerChannelFd[0],PayloadConsumerChannelFd[1]);
    PayloadProducerChannelFd[0] = uv_fs_open((uv_loop_t*)g_uvloop, &file_req, "\\\\.\\Pipe\\PayloadProducerChannelFd0", O_CREAT | O_RDWR, 0644, NULL);
    PayloadProducerChannelFd[1] = uv_fs_open((uv_loop_t*)g_uvloop, &file_req, "\\\\.\\Pipe\\PayloadProducerChannelFd1", O_CREAT | O_RDWR, 0644, NULL);
    MS_lOGD("pipe Create Pair PayloadProducerChannelFd[0]=%d PayloadProducerChannelFd[1]=%d ",PayloadProducerChannelFd[0],PayloadProducerChannelFd[1]);
#endif
    /*
    char str[34]={0};
    strcpy(str,"test");
    write(ProducerChannelFd[1], str, sizeof(str));
    char buf[34]={0};
    read(ProducerChannelFd[0], buf, 34);
*/
    
//    json response = {
//                   {"response", true},
//                   {"ok", true},
//        {"data", json({})}
//    };
//    MS_lOGD("response = %s",response.dump().c_str());
    // Initialize libuv stuff (we need it for the Channel).
    DepLibUV::ClassInit();
   
    server.setConfig(config);

    server.initMediasoup();

    MS_lOGD("initWorker ProducerChannelFd[0]=%d,ConsumerChannelFd[1]=%d,PayloadProducerChannelFd[0]=%d,PayloadConsumerChannelFd[1]=%d",ProducerChannelFd[0],ConsumerChannelFd[1],PayloadProducerChannelFd[0],PayloadConsumerChannelFd[1]);
    server.initWorker(ProducerChannelFd[0],ConsumerChannelFd[1],PayloadProducerChannelFd[0],PayloadConsumerChannelFd[1]);

    std::thread t1([&]() {
        server.run();
    });
    t1.detach();


    worker_init(argc,argv);
   
   
    
    getchar();

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
