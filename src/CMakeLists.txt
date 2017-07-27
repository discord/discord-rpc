include_directories(${PROJECT_SOURCE_DIR}/include)

option(ENABLE_IO_THREAD "Start up a separate I/O thread, otherwise I'd need to call an update function" ON)

if (${ENABLE_IO_THREAD} EQUAL OFF)
    add_definitions(-DDISCORD_DISABLE_IO_THREAD)
endif (${ENABLE_IO_THREAD} EQUAL OFF)

set(BASE_RPC_SRC ${PROJECT_SOURCE_DIR}/include/discord-rpc.h discord-rpc.cpp discord-register.cpp rpc_connection.h rpc_connection.cpp serialization.h serialization.cpp connection.h backoff.h)

if(WIN32)
    add_library(discord-rpc STATIC ${BASE_RPC_SRC} connection_win.cpp)
    target_compile_options(discord-rpc PRIVATE /W4)
endif(WIN32)

if(UNIX)
    add_library(discord-rpc STATIC ${BASE_RPC_SRC} connection_unix.cpp)
endif(UNIX)

target_include_directories(discord-rpc PRIVATE ${RAPIDJSON}/include)

add_dependencies(discord-rpc clangformat)

# install

install(
    TARGETS discord-rpc
    EXPORT "discord-rpc"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    INCLUDES DESTINATION "include"
)

install(
    FILES
        "../include/discord-rpc.h"
    DESTINATION "include"
)
