#pragma once

#include "Singleton.hpp"
//#include "spdlog/spdlog.h"

namespace mediasoup
{

class Log : public mediasoup::Singleton<Log> {
public:
    Log();
    ~Log();

    template <typename... Args>
    void trace(const Args&... args) {
	    //spdlog::trace(args...);
    }

	template <typename... Args>
    void debug(const Args&... args) {
	    //spdlog::debug(args...);
        
    }

    template <typename... Args>
    void info(const Args&... args) {
	    //spdlog::info(args...);
    }

    template <typename... Args>
    void warn(const Args&... args) {
	    //spdlog::warn(args...);
    }

    template <typename... Args>
    void error(const Args&... args) {
	    //spdlog::error(args...);
    }
};
#if 1

#define MS_lOGD(...) { \
fprintf(stderr, "%s: Line %d:\t", __FILE__, __LINE__); \
fprintf(stderr, __VA_ARGS__); \
fprintf(stderr,"\n"); \
}
#define MS_lOGI(...) { \
fprintf(stderr, "%s: Line %d:\t", __FILE__, __LINE__); \
fprintf(stderr, __VA_ARGS__); \
fprintf(stderr,"\n"); \
}
#define MS_lOGW(...) { \
fprintf(stderr, "%s: Line %d:\t", __FILE__, __LINE__); \
fprintf(stderr, __VA_ARGS__); \
fprintf(stderr,"\n"); \
}
#define MS_lOGE(...) { \
fprintf(stderr, "%s: Line %d:\t", __FILE__, __LINE__); \
fprintf(stderr, __VA_ARGS__); \
fprintf(stderr,"\n"); \
}
#define MS_THROW_lOG(...) { \
fprintf(stderr, "%s: Line %d:\t", __FILE__, __LINE__); \
fprintf(stderr, __VA_ARGS__); \
fprintf(stderr,"\n"); \
}
#define MS_lOGF(...) {}
//mediasoup::Log::GetInstance().debug(__FUNCTION__)
#else
#define MS_lOGD(...) mediasoup::Log::GetInstance().debug(__VA_ARGS__)
#define MS_lOGI(...) mediasoup::Log::GetInstance().info(__VA_ARGS__)
#define MS_lOGW(...) mediasoup::Log::GetInstance().warn(__VA_ARGS__)
#define MS_lOGE(...) mediasoup::Log::GetInstance().error(__VA_ARGS__)
#define MS_lOGF(...) mediasoup::Log::GetInstance().debug(__VA_ARGS__)
//mediasoup::Log::GetInstance().debug(__FUNCTION__)
#endif
#define MS_ASSERT_RV_LOGI(x, r, ...) \
  do {                           \
    if (!(x)) {                  \
      MS_lOGI(__VA_ARGS__);   \
      return (r);                \
    }                            \
  } while (0);

#define MS_ASSERT_RV_LOGW(x, r, ...) \
  do {                           \
    if (!(x)) {                  \
      MS_lOGW(__VA_ARGS__);   \
      return (r);                \
    }                            \
  } while (0);

#define MS_ASSERT_RV_LOGE(x, r, ...) \
  do {                           \
    if (!(x)) {                  \
      MS_lOGE(__VA_ARGS__);   \
      return (r);                \
    }                            \
  } while (0);

#define MS_ASSERT_R_LOGI(x, ...) \
  do {                           \
    if (!(x)) {                  \
      MS_lOGI(__VA_ARGS__);   \
      return;                    \
    }                            \
  } while (0);

#define MS_ASSERT_R_LOGW(x, ...) \
  do {                           \
    if (!(x)) {                  \
      MS_lOGW(__VA_ARGS__);   \
      return;                    \
    }                            \
  } while (0);

#define MS_ASSERT_R_LOGE(x, ...) \
  do {                           \
    if (!(x)) {                  \
      MS_lOGE(__VA_ARGS__);   \
      return;                    \
    }                            \
  } while (0);

}
