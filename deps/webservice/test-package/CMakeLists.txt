cmake_minimum_required(VERSION 3.12.4)

project(test_webservice_package)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(webservice REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} test.cpp)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${webservice_INCLUDE_DIR} ${Boost_INCLUDE_DIR})

target_link_directories(${PROJECT_NAME}
    PRIVATE ${webservice_LIB_DIR} ${Boost_LIB_DIR})

target_link_libraries(${PROJECT_NAME}
    PRIVATE webservice ${Boost_LIBRARIES} Threads::Threads)
