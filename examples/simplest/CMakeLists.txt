include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(simplest-client simple.c)
target_link_libraries(simplest-client discord-rpc-simple)
