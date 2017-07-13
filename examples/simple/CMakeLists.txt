include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(simple-client-async simple.c)
target_link_libraries(simple-client-async discord-rpc)
