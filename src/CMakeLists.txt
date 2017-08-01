include_directories(${PROJECT_SOURCE_DIR}/include)

option(ENABLE_IO_THREAD "Start up a separate I/O thread, otherwise I'd need to call an update function" ON)
option(BUILD_DYNAMIC_LIB "Build library as a DLL" OFF)

if (NOT ${ENABLE_IO_THREAD})
    add_definitions(-DDISCORD_DISABLE_IO_THREAD)
endif (NOT ${ENABLE_IO_THREAD})

set(BASE_RPC_SRC
    ${PROJECT_SOURCE_DIR}/include/discord-rpc.h
    discord-rpc.cpp
    discord-register.cpp
    rpc_connection.h
    rpc_connection.cpp
    serialization.h
    serialization.cpp
    connection.h
    backoff.h
)

if (${BUILD_DYNAMIC_LIB})
    set(RPC_LIBRARY_TYPE SHARED)
    set(BASE_RPC_SRC ${BASE_RPC_SRC} dllmain.cpp)
else(${BUILD_DYNAMIC_LIB})
    set(RPC_LIBRARY_TYPE STATIC)
endif(${BUILD_DYNAMIC_LIB})

if(WIN32)
    add_library(discord-rpc ${RPC_LIBRARY_TYPE} ${BASE_RPC_SRC} connection_win.cpp)
    target_compile_options(discord-rpc PRIVATE /W4)
endif(WIN32)

if(UNIX)
    add_library(discord-rpc ${RPC_LIBRARY_TYPE} ${BASE_RPC_SRC} connection_unix.cpp)
    target_link_libraries(discord-rpc PUBLIC pthread)
    target_compile_options(discord-rpc PRIVATE -g -Wall)
endif(UNIX)

target_include_directories(discord-rpc PRIVATE ${RAPIDJSON}/include)

if (${BUILD_DYNAMIC_LIB})
    target_compile_definitions(discord-rpc PUBLIC -DDISCORD_DYNAMIC_LIB)
    target_compile_definitions(discord-rpc PRIVATE -DDISCORD_BUILDING_SDK)
endif(${BUILD_DYNAMIC_LIB})

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
