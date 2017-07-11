include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(discord-rpc-simple STATIC ${PROJECT_SOURCE_DIR}/include/discord-rpc.h discord-rpc-simple.cpp)

set(BASE_RPC_SRC ${PROJECT_SOURCE_DIR}/include/discord-rpc.h discord-rpc.cpp yolojson.h connection.h)

if(WIN32)
    add_library(discord-rpc-sync STATIC ${BASE_RPC_SRC} connection_win_sync.cpp)
    add_library(discord-rpc STATIC ${BASE_RPC_SRC} connection_win.cpp)
endif(WIN32)
