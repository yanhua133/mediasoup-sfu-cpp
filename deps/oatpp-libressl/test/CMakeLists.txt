add_executable(module-tests
        oatpp-libressl/tests.cpp
        oatpp-libressl/FullTest.cpp
        oatpp-libressl/FullTest.hpp
        oatpp-libressl/FullAsyncTest.cpp
        oatpp-libressl/FullAsyncTest.hpp
        oatpp-libressl/FullAsyncClientTest.cpp
        oatpp-libressl/FullAsyncClientTest.hpp
        oatpp-libressl/app/Controller.hpp
        oatpp-libressl/app/AsyncController.hpp
        oatpp-libressl/app/Client.hpp
        oatpp-libressl/app/DTOs.hpp
        )

#################################################################
## define certificates path

add_definitions(
        -DCERT_PEM_PATH="${CMAKE_CURRENT_LIST_DIR}/../utility/cert/test_key.pem"
        -DCERT_CRT_PATH="${CMAKE_CURRENT_LIST_DIR}/../utility/cert/test_cert.crt"
)

#################################################################

set_target_properties(module-tests PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)

target_include_directories(module-tests
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

if(OATPP_MODULES_LOCATION STREQUAL OATPP_MODULES_LOCATION_EXTERNAL)
    add_dependencies(module-tests ${LIB_OATPP_EXTERNAL})
endif()

add_dependencies(module-tests ${OATPP_THIS_MODULE_NAME})

target_link_oatpp(module-tests)

target_link_libraries(module-tests
        PRIVATE ${OATPP_THIS_MODULE_NAME}
)

add_test(module-tests module-tests)
