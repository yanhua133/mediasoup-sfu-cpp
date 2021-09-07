# Install script for directory: /Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/release")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/build/liblibwebrtc.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liblibwebrtc.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liblibwebrtc.a")
    execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liblibwebrtc.a")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/system_wrappers/source/field_trial.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/rtc_base/rate_statistics.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/rtc_base/experiments/field_trial_parser.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/rtc_base/experiments/field_trial_units.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/rtc_base/experiments/alr_experiment.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/rtc_base/experiments/rate_control_settings.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/rtc_base/network/sent_packet.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/rtc_base/units/unit_base.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/rtc_base/constructor_magic.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/rtc_base/numerics/safe_minmax.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/rtc_base/numerics/safe_conversions.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/rtc_base/numerics/safe_conversions_impl.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/rtc_base/numerics/percentile_filter.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/rtc_base/numerics/safe_compare.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/rtc_base/system/unused.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/rtc_base/type_traits.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/call/rtp_transport_controller_send.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/call/rtp_transport_controller_send_interface.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/api/transport/webrtc_key_value_config.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/api/transport/network_types.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/api/transport/bitrate_settings.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/api/transport/network_control.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/api/transport/field_trial_based_config.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/api/transport/goog_cc_factory.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/api/bitrate_constraints.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/api/units/frequency.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/api/units/data_size.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/api/units/time_delta.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/api/units/data_rate.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/api/units/timestamp.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/api/network_state_predictor.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/include/module_common_types_public.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/pacing/interval_budget.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/pacing/paced_sender.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/pacing/packet_router.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/pacing/bitrate_prober.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/remote_bitrate_estimator/inter_arrival.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/remote_bitrate_estimator/overuse_detector.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/remote_bitrate_estimator/overuse_estimator.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/remote_bitrate_estimator/include/bwe_defines.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/remote_bitrate_estimator/aimd_rate_control.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/remote_bitrate_estimator/remote_bitrate_estimator_abs_send_time.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/remote_bitrate_estimator/include/remote_bitrate_estimator.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/rtp_rtcp/include/rtp_rtcp_defines.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/rtp_rtcp/source/rtp_packet/transport_feedback.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/bitrate_controller/loss_based_bandwidth_estimation.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/bitrate_controller/send_side_bandwidth_estimation.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/congestion_controller/goog_cc/bitrate_estimator.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/congestion_controller/goog_cc/link_capacity_estimator.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/congestion_controller/goog_cc/median_slope_estimator.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/congestion_controller/goog_cc/probe_controller.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/congestion_controller/goog_cc/trendline_estimator.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/congestion_controller/goog_cc/goog_cc_network_control.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/congestion_controller/goog_cc/delay_increase_detector_interface.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/congestion_controller/goog_cc/acknowledged_bitrate_estimator.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/congestion_controller/goog_cc/congestion_window_pushback_controller.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/congestion_controller/goog_cc/delay_based_bwe.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/congestion_controller/goog_cc/probe_bitrate_estimator.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/congestion_controller/goog_cc/alr_detector.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/congestion_controller/rtp/send_time_history.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/congestion_controller/rtp/transport_feedback_adapter.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/modules/congestion_controller/rtp/control_handler.h"
    "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/libwebrtc/mediasoup_helpers.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/yanhua/Documents/work/github/mediasoup-sfu-cpp/deps/libwebrtc/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
