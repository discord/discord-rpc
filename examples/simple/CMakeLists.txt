include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(simple-async-client simple.c)
target_link_libraries(simple-async-client discord-rpc)
