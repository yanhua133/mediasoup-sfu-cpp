
add_library(${OATPP_THIS_MODULE_NAME}
        oatpp-openssl/Config.cpp
        oatpp-openssl/Config.hpp
        oatpp-openssl/Connection.cpp
        oatpp-openssl/Connection.hpp
        oatpp-openssl/client/ConnectionProvider.cpp
        oatpp-openssl/client/ConnectionProvider.hpp
        oatpp-openssl/server/ConnectionProvider.cpp
        oatpp-openssl/server/ConnectionProvider.hpp
        oatpp-openssl/configurer/CertificateChainFile.cpp
        oatpp-openssl/configurer/CertificateChainFile.hpp
        oatpp-openssl/configurer/CertificateFile.cpp
        oatpp-openssl/configurer/CertificateFile.hpp
        oatpp-openssl/configurer/ContextConfigurer.hpp
        oatpp-openssl/configurer/PrivateKeyFile.cpp
        oatpp-openssl/configurer/PrivateKeyFile.hpp
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
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}
        ${OPENSSL_INCLUDE_DIR}
        >
)

target_link_libraries(${OATPP_THIS_MODULE_NAME}
        # PUBLIC OpenSSL::SSL
        # PUBLIC OpenSSL::Crypto
)

#######################################################################################################
## install targets

if(OATPP_INSTALL)
    include("../cmake/module-install.cmake")
endif()
