include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(simple-client simple.c)
target_link_libraries(simple-client discord-rpc)
