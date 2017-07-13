include_directories(${PROJECT_SOURCE_DIR}/include)

set(BASE_RPC_SRC ${PROJECT_SOURCE_DIR}/include/discord-rpc.h discord-rpc.cpp yolojson.h connection.h)

if(WIN32)
    add_library(discord-rpc-simple STATIC ${PROJECT_SOURCE_DIR}/include/discord-rpc.h discord-rpc-simple.cpp)

    add_library(discord-rpc-sync STATIC ${BASE_RPC_SRC} connection_win_sync.cpp)
    target_include_directories(discord-rpc-sync PRIVATE ${RAPIDJSON}/include)

    add_library(discord-rpc STATIC ${BASE_RPC_SRC} connection_win.cpp)
    target_include_directories(discord-rpc PRIVATE ${RAPIDJSON}/include)
endif(WIN32)

if(UNIX)
    add_library(discord-rpc STATIC ${BASE_RPC_SRC} connection_unix.cpp)
    target_include_directories(discord-rpc PRIVATE ${RAPIDJSON}/include)
endif(UNIX)
