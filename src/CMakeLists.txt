include_directories(${PROJECT_SOURCE_DIR}/include)

set(BASE_RPC_SRC ${PROJECT_SOURCE_DIR}/include/discord-rpc.h discord-rpc.cpp rpc_connection.h rpc_connection.cpp yolojson.h connection.h backoff.h)

if(WIN32)
    add_library(discord-rpc STATIC ${BASE_RPC_SRC} connection_win.cpp)
endif(WIN32)

if(UNIX)
    add_library(discord-rpc STATIC ${BASE_RPC_SRC} connection_unix.cpp)
endif(UNIX)

target_include_directories(discord-rpc PRIVATE ${RAPIDJSON}/include)
