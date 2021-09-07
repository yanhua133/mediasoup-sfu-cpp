#include "Log.hpp"

#include <stdarg.h>
#include <chrono>

//#include "spdlog/sinks/basic_file_sink.h"

namespace mediasoup
{

Log::Log() {
#ifdef NDEBUG
    // Create a daily logger - a new file is created every day on 12:00am
    //auto fileLogger = spdlog::daily_logger_mt("mediasoup", "logs/mediasoup.log", 12, 0);
    // Set the default logger to file logger    
    //spdlog::set_default_logger(fileLogger);
   // spdlog::set_level(spdlog::level::info);
#else
   // spdlog::set_level(spdlog::level::trace);
#endif
   // spdlog::flush_every(std::chrono::seconds(2));
   // spdlog::flush_on(spdlog::level::err);
	// https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
   // spdlog::set_pattern("[%H:%M:%S %z] %P %t [%^- %L -%$] %v");
}

Log::~Log() {
}

}