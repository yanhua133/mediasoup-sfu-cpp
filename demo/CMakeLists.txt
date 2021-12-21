cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME "sfu-demo")

project(${PROJECT_NAME})
message("\n=========== ${PROJECT_NAME} Build Configuration ===========\n")
message(STATUS "PROJECT_INCLUDE_DIR : " ${PROJECT_INCLUDE_DIR})
message(STATUS "PROJECT_LIB_DIR : " ${PROJECT_LIB_DIR})

# C++ standard requirements.
set(CMAKE_CXX_STANDAR 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

IF(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ENDIF()

IF (WIN32)
	#SET(CMAKE_C_COMPILER "c:/Program Files/LLVM/bin/clang")
	#SET(CMAKE_CXX_COMPILER "c:/Program Files/LLVM/bin/clang++")
ELSEIF (APPLE)

ELSEIF (UNIX)

  message(STATUS "This is BINARY dir " ${PROJECT_BINARY_DIR})
  message(STATUS "This is SOURCE dir " ${PROJECT_SOURCE_DIR})
  message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
  message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
ENDIF ()
message("")

set(sources
  main.cpp
  Server.hpp
  Server.cpp
  Config.hpp
  Config.cpp
  Transfmt.cpp
  Transfmt.hpp
  AppComponent.hpp
  controller/RoomsController.hpp
  controller/StatisticsController.hpp
  dto/ConfigDto.hpp
  dto/DTOs.hpp
  rooms/Lobby.cpp
  rooms/Lobby.hpp
  rooms/Message.cpp
  rooms/Message.h
  rooms/Peer.cpp
  rooms/Peer.hpp
  rooms/Room.cpp
  rooms/Room.hpp
  utils/Statistics.cpp
  utils/Statistics.hpp 
  utils/util.cpp
  utils/util.h
)

add_executable(${PROJECT_NAME} ${sources} )

#[[
TODO:

# FIPS.
      [ 'openssl_fips != ""', {
        'defines': [ 'BUD_FIPS_ENABLED=1' ]
      }],
# Platform-specifics.

      [ 'OS == "mac" and mediasoup_asan == "true"', {
        'xcode_settings':
        {
          'OTHER_CFLAGS': [ '-fsanitize=address' ],
          'OTHER_LDFLAGS': [ '-fsanitize=address' ]
        }
      }],

      [ 'OS == "linux" and mediasoup_asan == "true"', {
        'cflags': [ '-fsanitize=address' ],
        'ldflags': [ '-fsanitize=address' ]
      }],
# Dependency-specifics.

      [ 'sctp_debug == "true"', {
        'defines': [ 'SCTP_DEBUG' ]
      }]
[ 'OS in "linux freebsd"', {
        'ldflags': [ '-Wl,--whole-archive <(libopenssl) -Wl,--no-whole-archive' ]
      }],
[ 'OS == "win"', {

        # Handle multi files with same name.
        # https://stackoverflow.com/a/22936230/2085408
        # https://docs.microsoft.com/en-us/dotnet/api/microsoft.visualstudio.vcprojectengine.vcclcompilertool.objectfile?view=visualstudiosdk-2017#Microsoft_VisualStudio_VCProjectEngine_VCCLCompilerTool_ObjectFile
        'msvs_settings': {
          'VCCLCompilerTool': { 'ObjectFile': ['$(IntDir)\%(RelativeDir)\%(Filename).obj'], },
        },

        # Output Directory setting for msvc.
        # https://github.com/nodejs/node-gyp/issues/1242#issuecomment-310921441
        'msvs_configuration_attributes': {
          'OutputDirectory': '$(SolutionDir)\\out\\$(Configuration)\\'
        }
      }],
]]

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.
 


if(IS_BIG_ENDIAN)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "MS_BIG_ENDIAN")
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE "MS_LITTLE_ENDIAN")
endif()


IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_compile_definitions(${PROJECT_NAME} 
        PRIVATE 
        _POSIX_C_SOURCE=200112
        _GNU_SOURCE
    )
    #include_directories(${LLVM_INCLUDE_DIRS})
    #add_definitions(${LLVM_DEFINITIONS})
    # Find the libraries that correspond to the LLVM components
    # that we wish to use
    #llvm_map_components_to_libnames(llvm_libs support core irreader)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-dynamic")
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_include_directories(${PROJECT_NAME}
    PRIVATE
    ../deps/getopt/src
)
ELSE()

ENDIF(CMAKE_SYSTEM_NAME MATCHES "Linux")

IF(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -g -gdwarf-2 -Wall -Wendif-labels -W -Wno-unused-parameter -Wall -Wextra -Wno-unused-parameter -std=c++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC")
ENDIF()
message(STATUS "CMAKE_CXX_FLAGS : " ${CMAKE_CXX_FLAGS})

IF(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set_property(TARGET ${PROJECT_NAME}
        PROPERTY XCODE_ATTRIBUTE_WARNING_CFLAGS
        -std=c++11
        PROPERTY XCODE_ATTRIBUTE_OTHER_CPLUSPLUSFLAGS

    )
ENDIF()

# Public (interface) headers from dependencies.
target_include_directories(${PROJECT_NAME} PUBLIC
	${PROJECT_INCLUDE_DIR}
  ${WORKER_INCLUDE_DIR}
  ${LIB_WEBRTC_INCLUDE_DIR}
  ${MEDIASOUP_INCLUDE_DIR}
  ${PROTOO_INCLUDE_DIR}
	
)
LINK_DIRECTORIES(
	${PROJECT_LIB_DIR}
)


IF (WIN32)
  target_link_libraries(${PROJECT_NAME}
      worker
      libwebrtc
      ${PROJECT_LIB_DIR}/oatpp.lib
      ${PROJECT_LIB_DIR}/oatpp-websocket.lib
      ${PROJECT_LIB_DIR}/oatpp-openssl.lib
      ${PROJECT_LIB_DIR}/netstring.lib
      ${PROJECT_LIB_DIR}/uv_a.lib
      ${PROJECT_LIB_DIR}/libssl.lib
      ${PROJECT_LIB_DIR}/libcrypto.lib
      ${PROJECT_LIB_DIR}/srtp2.lib
      ${PROJECT_LIB_DIR}/usrsctp.lib
      ${PROJECT_LIB_DIR}/absl_internal_throw_delegate.lib
      ${PROJECT_LIB_DIR}/absl_strings.lib
      ${PROJECT_LIB_DIR}/absl_optional.lib
      ${PROJECT_LIB_DIR}/absl_base.lib
      ${PROJECT_LIB_DIR}/absl_bad_optional_access.lib
      ${PROJECT_LIB_DIR}/absl_bad_variant_access.lib
      ${PROJECT_LIB_DIR}/absl_internal_bad_any_cast_impl.lib
      ${PROJECT_LIB_DIR}/sdptransform.lib
      ${PROJECT_LIB_DIR}/getopt.lib
      mediasoup
      ws2_32
      userenv
      
       # LibreSSL libraries
      # LibreSSL::TLS
      # LibreSSL::SSL
      # LibreSSL::Crypto
  )
  
ELSE ()

target_link_libraries(${PROJECT_NAME}
    worker
    libwebrtc
    ${PROJECT_LIB_DIR}/liboatpp.a
    ${PROJECT_LIB_DIR}/liboatpp-websocket.a
    ${PROJECT_LIB_DIR}/liboatpp-openssl.a
    ${PROJECT_LIB_DIR}/libnetstring.a
    ${PROJECT_LIB_DIR}/libuv_a.a
    ${PROJECT_LIB_DIR}/libssl.a
    ${PROJECT_LIB_DIR}/libcrypto.a
    ${PROJECT_LIB_DIR}/libsrtp2.a
    ${PROJECT_LIB_DIR}/libusrsctp.a
    ${PROJECT_LIB_DIR}/libabsl_internal_throw_delegate.a
    ${PROJECT_LIB_DIR}/libabsl_strings.a
    ${PROJECT_LIB_DIR}/libabsl_optional.a
    ${PROJECT_LIB_DIR}/libabsl_bad_optional_access.a
    ${PROJECT_LIB_DIR}/libabsl_bad_variant_access.a
    ${PROJECT_LIB_DIR}/libabsl_internal_bad_any_cast_impl.a
    dl
    ${PROJECT_LIB_DIR}/libsdptransform.a
    mediasoup
    ${PROJECT_LIB_DIR}/libuv_a.a

     # LibreSSL libraries
    #  LibreSSL::TLS
    #  LibreSSL::SSL
    #  LibreSSL::Crypto
    
)
ENDIF()
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

