cmake_minimum_required(VERSION 3.12.4)

project(protoo LANGUAGES CXX)


# interface target
#find_package(Boost REQUIRED)

set(DEFAULT_BUILD_TYPE "Release")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(Boost_INCLUDE_DIR ${PROJECT_INCLUDE_DIR})
set(Boost_SYSTEM_LIBRARY ${PROJECT_LIB_DIR})
message(STATUS "Boost_INCLUDE_DIR : " ${Boost_INCLUDE_DIR})
message(STATUS "Boost_SYSTEM_LIBRARY : " ${Boost_SYSTEM_LIBRARY})
IF(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
ENDIF()

file(GLOB SOURCE_FILES 
        Transport/HttpTransport.cpp
        Transport/HttpTransport.h
        Transport/WebSocketTransport.cpp
        Transport/WebSocketTransport.h
        Transport/WebSocketServer.cpp
        Transport/WebSocketServer.h
        Room.cpp
        Room.h
        Peer.cpp
        Peer.h
        Message.cpp
        Message.h
        utils/EventEmitter.hpp
        utils/utils.cpp
        utils/utils.h
        utils/PeerTimer.cpp
        utils/PeerTimer.h
		    utils/Log.hpp
      )

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils )
target_include_directories(${PROJECT_NAME}
    SYSTEM PUBLIC $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>)
target_include_directories(${PROJECT_NAME}
    SYSTEM PUBLIC ${Boost_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC
	${PROJECT_INCLUDE_DIR}
  ${WORKER_INCLUDE_DIR}
  ${LIB_WEBRTC_INCLUDE_DIR}	
  ${PROTOO_INCLUDE_DIR}
  ${MEDIASOUP_INCLUDE_DIR}
)

# target_link_directories(${PROJECT_NAME}
#     PUBLIC $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/lib>)
target_link_libraries(${PROJECT_NAME}
    PUBLIC ${Boost_SYSTEM_LIBRARY})

install(TARGETS ${PROJECT_NAME} DESTINATION lib)