cmake_minimum_required(VERSION 3.12.4)

project(webservice LANGUAGES CXX)


# interface target
#find_package(Boost REQUIRED)

set(DEFAULT_BUILD_TYPE "Release")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(Boost_INCLUDE_DIR ${PROJECT_INCLUDE_DIR})
set(Boost_SYSTEM_LIBRARY ${PROJECT_LIB_DIR})
message(STATUS "Boost_INCLUDE_DIR : " ${Boost_INCLUDE_DIR})
message(STATUS "Boost_SYSTEM_LIBRARY : " ${Boost_SYSTEM_LIBRARY})

file(GLOB SOURCE_FILES src/*.cpp include/webservice/*.hpp)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_include_directories(${PROJECT_NAME}
    SYSTEM PUBLIC $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>)
target_include_directories(${PROJECT_NAME}
    SYSTEM PUBLIC ${Boost_INCLUDE_DIR})
target_link_directories(${PROJECT_NAME}
    PUBLIC $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/lib>)
target_link_libraries(${PROJECT_NAME}
    PUBLIC ${Boost_SYSTEM_LIBRARY})

# Setup package config
set(INCLUDE_INSTALL_DIR include)
set(LIB_INSTALL_DIR lib)
set(CONFIG_PACKAGE_INSTALL_DIR ${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME})

include(CMakePackageConfigHelpers)
configure_package_config_file(${PROJECT_NAME}-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    INSTALL_DESTINATION ${LIB_INSTALL_DIR}/${PROJECT_NAME}/cmake
    PATH_VARS INCLUDE_INSTALL_DIR LIB_INSTALL_DIR)

# Install target
install(DIRECTORY include/${PROJECT_NAME}
    DESTINATION ${INCLUDE_INSTALL_DIR})
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib COMPONENT libraries)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    DESTINATION ${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME})


### TODO: Add examples and tests with cmake!
# # other subdirectories
# # only add if not inside add_subdirectory()
# option(WEBSERVICE_BUILD_EXAMPLES "build examples" OFF)
# if(${WEBSERVICE_BUILD_EXAMPLES})
#     add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/examples)
# endif()
#
# option(WEBSERVICE_BUILD_TESTS "build tests" OFF)
# if(${WEBSERVICE_BUILD_TESTS})
#     enable_testing()
#     find_package(GTest REQUIRED)
#     add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
#     add_test(NAME tests COMMAND tests)
# endif()
