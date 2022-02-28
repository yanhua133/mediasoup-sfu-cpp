cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME "mediasoup")
project(${PROJECT_NAME})

# C++ standard requirements.
set(CMAKE_CXX_STANDAR 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
IF(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -gdwarf-2 -Wall -Wendif-labels -W -Wno-unused-parameter -Wall -Wextra -Wno-unused-parameter -std=c++14")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
ENDIF()
message(STATUS "CMAKE_CXX_FLAGS : " ${CMAKE_CXX_FLAGS})
set(mediasoup_headers
  Version.hpp
  common/AutoRelease.hpp
  common/CommonObserver.hpp
  Singleton.hpp
  util/Process.hpp
  Log.hpp
  uv/UVPipeWrapper.hpp
  IMediasoup.hpp
  Mediasoup.hpp  
  Promise.hpp
  IWorker.hpp
  WorkerAgent.hpp
  RtpParameters.hpp
  SupportedRtpCapabilities.hpp
  ChannelAgent.hpp
  Consumer.hpp
  DataConsumer.hpp
  DataProducer.hpp
  DirectTransport.hpp
  EventEmitterEx.hpp
  EnhancedEventEmitter.hpp
  errors.hpp
  PipeTransport.hpp
  PlainTransport.hpp
  Producer.hpp
  Router.hpp
  SctpParameters.hpp
  SrtpParameters.hpp
  Transport.hpp
  WebRtcTransport.hpp
  AudioLevelObserver.hpp
  ortc.hpp
  RtpObserver.hpp
  scalabilityModes.hpp
  uuid.hpp
  util/event_emitter.hpp
  util/composition.hpp
  util/event.hpp
  util/subscription.hpp
  sdp/MediaSection.hpp
  sdp/RemoteSdp.hpp
  sdp/SdpUtils.hpp
  MediaSoupClientErrors.hpp
  LoggerC.hpp
  sdp/common_types.h
  sdp/h264_profile_level_id.h
  sdp/h264_profile_level_id.cc
  sdp/rtc_export.h
  sdp/SdpOffer.hpp
  UtilsC.hpp
  util/timer.hpp
  PayloadChannelAgent.hpp
)

set(mediasoup_sources
  util/Process.cpp
  Log.cpp
  uv/UVPipeWrapper.cpp
  Mediasoup.cpp  
  WorkerAgent.cpp
  SupportedRtpCapabilities.cpp
  ChannelAgent.cpp
  PayloadChannelAgent.cpp
  ortc.cpp
  uuid.cpp
  JsonConvert.cpp
  sdp/MediaSection.cpp
  sdp/RemoteSdp.cpp
  sdp/SdpUtils.cpp
  sdp/SdpOffer.cpp
  LoggerC.cpp
  util/timer.cpp
  scalabilityModes.cpp
)

add_library(mediasoup
  STATIC
  ${mediasoup_sources}
  ${mediasoup_headers}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  #uv
  #netstring
)
message(STATUS "CMAKE_CURRENT_SOURCE_DIR : " ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "PROJECT_INCLUDE_DIR : " ${PROJECT_INCLUDE_DIR})

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/common
  ${CMAKE_CURRENT_SOURCE_DIR}/util
  ${CMAKE_CURRENT_SOURCE_DIR}/uv
  ${CMAKE_CURRENT_SOURCE_DIR}/../deps/json/single_include/nlohmann
  ${CMAKE_CURRENT_SOURCE_DIR}/../deps/libwebrtc/deps/abseil-cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../deps/libsdptransform
  ${PROJECT_INCLUDE_DIR}
)

target_include_directories(${PROJECT_NAME}
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
#install(FILES ${mediasoup_headers} DESTINATION include)
