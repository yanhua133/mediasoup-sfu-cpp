
add_library(${OATPP_THIS_MODULE_NAME}
        oatpp-libressl/Callbacks.cpp
        oatpp-libressl/Callbacks.hpp
        oatpp-libressl/Config.cpp
        oatpp-libressl/Config.hpp
        oatpp-libressl/Connection.cpp
        oatpp-libressl/Connection.hpp
        oatpp-libressl/client/ConnectionProvider.cpp
        oatpp-libressl/client/ConnectionProvider.hpp
        oatpp-libressl/server/ConnectionProvider.cpp
        oatpp-libressl/server/ConnectionProvider.hpp
        oatpp-libressl/TLSObject.cpp
        oatpp-libressl/TLSObject.hpp
)

set_target_properties(${OATPP_THIS_MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)

if(OATPP_MODULES_LOCATION STREQUAL OATPP_MODULES_LOCATION_EXTERNAL)
    add_dependencies(${OATPP_THIS_MODULE_NAME} ${LIB_OATPP_EXTERNAL})
endif()

target_link_oatpp(${OATPP_THIS_MODULE_NAME})

target_include_directories(${OATPP_THIS_MODULE_NAME}
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(${OATPP_THIS_MODULE_NAME}
        PUBLIC LibreSSL::TLS
        PUBLIC LibreSSL::SSL
        PUBLIC LibreSSL::Crypto
)

#######################################################################################################
## install targets

if(OATPP_INSTALL)
    include("../cmake/module-install.cmake")
endif()
