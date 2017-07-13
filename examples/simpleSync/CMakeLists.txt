include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(simple-client-sync simpleSync.c)
target_link_libraries(simple-client-sync discord-rpc-sync)
