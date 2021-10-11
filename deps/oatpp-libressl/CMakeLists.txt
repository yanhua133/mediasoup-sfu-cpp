cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

###################################################################################################
## These variables are passed to oatpp-module-install.cmake script
## use these variables to configure module installation

set(OATPP_THIS_MODULE_NAME oatpp-libressl) ## name of the module (also name of folders in installation dirs)
set(OATPP_THIS_MODULE_VERSION "1.2.5") ## version of the module (also sufix of folders in installation dirs)
set(OATPP_THIS_MODULE_LIBRARIES oatpp-libressl) ## list of libraries to find when find_package is called
set(OATPP_THIS_MODULE_TARGETS oatpp-libressl) ## list of targets to install
set(OATPP_THIS_MODULE_DIRECTORIES oatpp-libressl) ## list of directories to install

###################################################################################################

project(${OATPP_THIS_MODULE_NAME}
        VERSION ${OATPP_THIS_MODULE_VERSION}
        LANGUAGES CXX
)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(OATPP_DIR_SRC "Path to oatpp module directory (sources)")
option(OATPP_DIR_LIB "Path to directory with liboatpp (directory containing ex: liboatpp.so or liboatpp.dynlib)")
option(OATPP_BUILD_TESTS "Build tests for this module" OFF)
option(OATPP_INSTALL "Install module binaries" ON)
option(LIBRESSL_INCLUDE_DIR "Path to oatpp module directory (LIBRESSL_INCLUDE_DIR)")
option(LIBRESSL_TLS_LIBRARY "Path to oatpp module directory (LIBRESSL_TLS_LIBRARY)")
option(LIBRESSL_SSL_LIBRARY "Path to oatpp module directory (LIBRESSL_SSL_LIBRARY)")
option(LIBRESSL_CRYPTO_LIBRARY "Path to oatpp module directory (LIBRESSL_CRYPTO_LIBRARY)")
option(LIBRESSL_LIBRARIES "Path to oatpp module directory (LIBRESSL_LIBRARIES)")
option(LIBRESSL_VERSION "Path to oatpp module directory (LIBRESSL_VERSION)")

message("LIBRESSL_INCLUDE_DIR=${LIBRESSL_INCLUDE_DIR}")
set(OATPP_MODULES_LOCATION "CUSTOM" CACHE STRING "Location where to find oatpp modules. can be [INSTALLED|EXTERNAL|CUSTOM]")

###################################################################################################
## get oatpp main module in specified location

set(OATPP_MODULES_LOCATION_INSTALLED INSTALLED)
set(OATPP_MODULES_LOCATION_EXTERNAL EXTERNAL)
set(OATPP_MODULES_LOCATION_CUSTOM CUSTOM)

if(OATPP_MODULES_LOCATION STREQUAL OATPP_MODULES_LOCATION_INSTALLED)

    message("Finding oatpp in location=INSTALLED")

    find_package(oatpp ${OATPP_THIS_MODULE_VERSION} REQUIRED)

    get_target_property(OATPP_INCLUDE oatpp::oatpp INTERFACE_INCLUDE_DIRECTORIES)
    message("OATPP_INCLUDE=${OATPP_INCLUDE}")

    get_target_property(OATPP_TEST_INCLUDE oatpp::oatpp-test INTERFACE_INCLUDE_DIRECTORIES)
    message("OATPP_TEST_INCLUDE=${OATPP_TEST_INCLUDE}")

elseif(OATPP_MODULES_LOCATION STREQUAL OATPP_MODULES_LOCATION_EXTERNAL)

    message("Finding oatpp in location=EXTERNAL")

    include(ExternalProject)

    set(MODULE_WAIT_DEPS ON)

    set(LIB_OATPP_EXTERNAL "lib_oatpp_external")
    ExternalProject_Add(${LIB_OATPP_EXTERNAL}
            GIT_REPOSITORY "https://github.com/oatpp/oatpp.git"
            GIT_TAG origin/master
            CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DOATPP_INSTALL=OFF -DOATPP_BUILD_TESTS=OFF
            INSTALL_COMMAND cmake -E echo "SKIP INSTALL '${LIB_OATPP_EXTERNAL}'"
    )

    ExternalProject_Get_Property(${LIB_OATPP_EXTERNAL} BINARY_DIR)
    set(OATPP_DIR_LIB ${BINARY_DIR}/src)

    ExternalProject_Get_Property(${LIB_OATPP_EXTERNAL} SOURCE_DIR)
    set(OATPP_DIR_SRC ${SOURCE_DIR}/src)

    message("OATPP_DIR_SRC --> '${OATPP_DIR_SRC}'")
    message("OATPP_DIR_LIB --> '${OATPP_DIR_LIB}'")

elseif(OATPP_MODULES_LOCATION STREQUAL OATPP_MODULES_LOCATION_CUSTOM)

    message("Finding oatpp in location=CUSTOM")

    message("OATPP_DIR_SRC --> '${OATPP_DIR_SRC}'")
    message("OATPP_DIR_LIB --> '${OATPP_DIR_LIB}'")

else()
    message("FATAL_ERROR Unknown location to find oatpp '${OATPP_MODULES_LOCATION}'")
endif()

if(OATPP_DIR_LIB)
    link_directories(${OATPP_DIR_LIB})
endif()

include(cmake/module-utils.cmake)

###################################################################################################
## get dependencies

message("\n############################################################################")
message("## ${OATPP_THIS_MODULE_NAME} module. Resolving dependencies...\n")

##############################
## Find Libressl dependency

include(FindPkgConfig)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/module")

#find_package(LibreSSL 3.0.0 REQUIRED)

message("LIBRESSL_INCLUDE_DIR=${LIBRESSL_INCLUDE_DIR}")
message("LIBRESSL_TLS_LIBRARY=${LIBRESSL_TLS_LIBRARY}")
message("LIBRESSL_SSL_LIBRARY=${LIBRESSL_SSL_LIBRARY}")
message("LIBRESSL_CRYPTO_LIBRARY=${LIBRESSL_CRYPTO_LIBRARY}")
message("LIBRESSL_LIBRARIES=${LIBRESSL_LIBRARIES}")
message("LIBRESSL_VERSION=${LIBRESSL_VERSION}")

message("\n############################################################################\n")

###################################################################################################
## define targets

add_subdirectory("src")

if(OATPP_BUILD_TESTS)
    enable_testing()
    add_subdirectory("test")
endif()
