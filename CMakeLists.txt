cmake_minimum_required (VERSION 3.7.0)
project (DiscordRPCExample)

# format
file(GLOB_RECURSE ALL_SOURCE_FILES
    examples/*.cpp examples/*.h examples/*.c
    include/*.h
    src/*.cpp src/*.h src/*.c
)

find_program(CLANG_FORMAT_CMD clang-format)

if (CLANG_FORMAT_CMD)
    add_custom_target(
        clangformat
        COMMAND clang-format
        -i -style=file -fallback-style=none
        ${ALL_SOURCE_FILES}
        DEPENDS
        ${ALL_SOURCE_FILES}
    )
else(CLANG_FORMAT_CMD)
    add_custom_target(
        clangformat
        COMMENT "no clang format"
    )
endif(CLANG_FORMAT_CMD)

# thirdparty stuff
execute_process( 
    COMMAND mkdir ${CMAKE_SOURCE_DIR}/thirdparty
    ERROR_QUIET
)

find_file(RAPIDJSONTEST NAMES rapidjson rapidjson-1.1.0 PATHS ${CMAKE_SOURCE_DIR}/thirdparty)
if (NOT RAPIDJSONTEST)
    message("no rapidjson, download")
    set(RJ_TAR_FILE ${CMAKE_SOURCE_DIR}/thirdparty/v1.1.0.tar.gz)
    file(DOWNLOAD https://github.com/miloyip/rapidjson/archive/v1.1.0.tar.gz ${RJ_TAR_FILE})
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf ${RJ_TAR_FILE}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/thirdparty
    )
    file(REMOVE ${RJ_TAR_FILE})
endif(NOT RAPIDJSONTEST)

find_file(RAPIDJSON NAMES rapidjson rapidjson-1.1.0 PATHS ${CMAKE_SOURCE_DIR}/thirdparty)

add_library(rapidjson STATIC IMPORTED ${RAPIDJSON})

# add subdirs

add_subdirectory(src)
add_subdirectory(examples/send-presence)
