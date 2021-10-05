cmake_minimum_required(VERSION 3.5)

# Store in CMAKE_DEB_HOST_ARCH var the current build architecture
execute_process(COMMAND
  dpkg-architecture
    -qDEB_HOST_ARCH
  OUTPUT_VARIABLE
    CMAKE_DEB_HOST_ARCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(${CMAKE_DEB_HOST_ARCH} MATCHES "arm64")
	set(CPU_TYPE "arm")
	message(STATUS "CMAKE_DEB_HOST_ARCH:${CMAKE_DEB_HOST_ARCH} CPU_TYPE=${CPU_TYPE}")  

elseif(${CMAKE_DEB_HOST_ARCH} MATCHES "i386")
	set(CPU_TYPE "x86")
	message(STATUS "CMAKE_DEB_HOST_ARCH:${CMAKE_DEB_HOST_ARCH} CPU_TYPE=${CPU_TYPE}") 
	set(CPU_TYPE "x86")
elseif(${CMAKE_DEB_HOST_ARCH} MATCHES "amd64")
	set(CPU_TYPE "x64")
	message(STATUS "CMAKE_DEB_HOST_ARCH:${CMAKE_DEB_HOST_ARCH} CPU_TYPE=${CPU_TYPE}") 	
	
else()
	set(CPU_TYPE "x64")
	message(STATUS "CMAKE_DEB_HOST_ARCH:${CMAKE_DEB_HOST_ARCH} CPU_TYPE=${CPU_TYPE}") 
endif()

IF (WIN32)
	#SET(CMAKE_C_COMPILER "c:/Program Files/LLVM/bin/clang")
	#SET(CMAKE_CXX_COMPILER "c:/Program Files/LLVM/bin/clang++")
ELSEIF (APPLE)

ELSEIF (UNIX)
  #find_package(LLVM REQUIRED CONFIG)
  message(STATUS "This is BINARY dir " ${PROJECT_BINARY_DIR})
  message(STATUS "This is SOURCE dir " ${PROJECT_SOURCE_DIR})
  #message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
  #message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
  #include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/llvm.toolchain.cmake)

ENDIF ()
message(STATUS "CMAKE_C_COMPILER:${CMAKE_C_COMPILER}") 
message(STATUS "CMAKE_CXX_COMPILER:${CMAKE_CXX_COMPILER}") 
set(PROJECT_NAME "mediasoup-sfu-cpp")
project(${PROJECT_NAME})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")

set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/release/include ${CMAKE_CURRENT_SOURCE_DIR}/worker/include)
set(PROJECT_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/release/lib)
set(WORKER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/worker/include)
set(LIB_WEBRTC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/libwebrtc ${CMAKE_CURRENT_SOURCE_DIR}/deps/libwebrtc/libwebrtc ${CMAKE_CURRENT_SOURCE_DIR}/deps/libwebrtc/deps/abseil-cpp)
set(PROJECT_RELEASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/release)
set(MEDIASOUP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mediasoup ${CMAKE_CURRENT_SOURCE_DIR}/mediasoup/utils)
set(PROTOO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/protoo ${CMAKE_CURRENT_SOURCE_DIR}/protoo/utils)

IF (WIN32)
  set(PROJECT_INCLUDE_DIR ${PROJECT_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/deps/boost/include)
  set(PROJECT_LIB_DIR ${PROJECT_LIB_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/${CPU_TYPE})
ENDIF ()

IF(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ENDIF()
message("\n=========== mediasoup-sfu-cpp Build Configuration ===========\n")
message(STATUS "PROJECT_INCLUDE_DIR : " ${PROJECT_INCLUDE_DIR})
message(STATUS "PROJECT_LIB_DIR : " ${PROJECT_LIB_DIR})

message("")





# add subdirectory
#add_subdirectory(deps/libuv)

# add openssl no need app 
set(WITH_APPS OFF CACHE INTERNAL "")
#add_subdirectory(deps/openssl)
unset(WITH_APPS)


#add_subdirectory(deps/catch)

set(TEST_APPS OFF CACHE INTERNAL "")
set(ENABLE_OPENSSL ON CACHE INTERNAL "")
#add_subdirectory(deps/libsrtp)
unset(TEST_APPS)
unset(ENABLE_OPENSSL)

#add_subdirectory(deps/netstring)

set(sctp_werror OFF CACHE INTERNAL "")
set(sctp_build_programs OFF CACHE INTERNAL "")
set(sctp_build_fuzzer OFF CACHE INTERNAL "")
set(sctp_link_programs_static OFF CACHE INTERNAL "")
#add_subdirectory(deps/usrsctp)
unset(sctp_werror)
unset(sctp_build_programs)
unset(sctp_build_fuzzer)
unset(sctp_link_programs_static)

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
#add_subdirectory(deps/json)
unset(JSON_BuildTests)
unset(JSON_Install)

IF(WIN32)
#add_subdirectory(deps/getopt)
ENDIF()

add_subdirectory(deps/oatpp)
add_subdirectory(deps/libwebrtc)
#add_subdirectory(deps/webservice)
add_subdirectory(protoo)
add_subdirectory(worker)
add_subdirectory(mediasoup)

add_subdirectory(demo)

