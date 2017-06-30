include_directories(${PROJECT_SOURCE_DIR}/include)
add_library(discord-rpc-sync STATIC ${PROJECT_SOURCE_DIR}/include/discord-rpc.h discord-rpc-sync.cpp)
