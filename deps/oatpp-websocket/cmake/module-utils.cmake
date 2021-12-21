macro(target_link_oatpp target)

    if(TARGET oatpp::oatpp) ## OATPP_MODULES_LOCATION == INSTALLED

        message("target_link_oatpp(${target}) to installed oatpp lib")

        target_link_libraries(${target}
                PRIVATE oatpp::oatpp
                PRIVATE oatpp::oatpp-test
        )

    else()

        message("target_link_oatpp(${target}) to found in provided path oatpp lib")

        target_include_directories(${target} PUBLIC $<BUILD_INTERFACE:${OATPP_DIR_SRC}>)
        #target_link_directories(${target} PRIVATE ${OATPP_DIR_LIB})
        target_link_libraries(${target}
                PRIVATE oatpp
                PRIVATE oatpp-test
        )

    endif()

endmacro()
