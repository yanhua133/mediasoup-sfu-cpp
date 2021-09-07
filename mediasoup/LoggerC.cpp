#define MSC_CLASS "LoggerC"

#include "LoggerC.hpp"
#include <iostream>

namespace mediasoupclient
{
	/* Class variables. */

	LoggerC::LogHandlerInterface* LoggerC::handler{ nullptr };
	char LoggerC::buffer[LoggerC::bufferSize];
	LoggerC::LogLevel LoggerC::logLevel = LoggerC::LogLevel::LOG_NONE;

	/* Class methods. */

	void LoggerC::SetLogLevel(LoggerC::LogLevel level)
	{
		LoggerC::logLevel = level;
	}

	void LoggerC::SetHandler(LogHandlerInterface* handler)
	{
		LoggerC::handler = handler;
	}

	void LoggerC::SetDefaultHandler()
	{
		LoggerC::handler = new LoggerC::DefaultLogHandler();
	}

	/* DefaultLogHandler */

	void LoggerC::DefaultLogHandler::OnLog(LogLevel /*level*/, char* payload, size_t /*len*/)
	{
		std::cout << payload << std::endl;
	}
} // namespace mediasoupclient
