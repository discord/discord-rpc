cmake_minimum_required (VERSION 3.7.0)
project (DiscordRPCExample)

execute_process( 
    COMMAND mkdir ${CMAKE_SOURCE_DIR}/thirdparty
    ERROR_QUIET
)

find_file(RAPIDJSON NAMES rapidjson rapidjson-1.1.0 PATHS ${CMAKE_SOURCE_DIR}/thirdparty)

if (NOT RAPIDJSON)
    message("no rapidjson, download")
    set(RJ_TAR_FILE ${CMAKE_SOURCE_DIR}/thirdparty/v1.1.0.tar.gz)
    file(DOWNLOAD https://github.com/miloyip/rapidjson/archive/v1.1.0.tar.gz ${RJ_TAR_FILE})
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf ${RJ_TAR_FILE}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/thirdparty
    )
    file(REMOVE ${RJ_TAR_FILE})
endif(NOT RAPIDJSON)

add_library(rapidjson STATIC IMPORTED ${RAPIDJSON})

add_subdirectory(src)
add_subdirectory(examples/send-presence)
