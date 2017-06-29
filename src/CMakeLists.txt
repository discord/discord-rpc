add_library(discord-rpc discord-rpc.cpp)
add_executable(test-client simple.cpp)
target_link_libraries(test-client discord-rpc)
