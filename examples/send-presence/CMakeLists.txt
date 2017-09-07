include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(send-presence send-presence.c)
target_link_libraries(send-presence discord-rpc)

install(
    TARGETS send-presence
    RUNTIME
        DESTINATION "bin"
        CONFIGURATIONS Release
)