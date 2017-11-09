include_directories(${PROJECT_SOURCE_DIR}/include)

option(ENABLE_IO_THREAD "Start up a separate I/O thread, otherwise I'd need to call an update function" ON)
option(BUILD_DYNAMIC_LIB "Build library as a DLL" OFF)

set(BASE_RPC_SRC
    ${PROJECT_SOURCE_DIR}/include/discord-rpc.h
    discord-rpc.cpp
    discord_register.h
    rpc_connection.h
    rpc_connection.cpp
    serialization.h
    serialization.cpp
    connection.h
    backoff.h
    msg_queue.h
)

if (${BUILD_DYNAMIC_LIB})
    set(RPC_LIBRARY_TYPE SHARED)
    if(WIN32)
        set(BASE_RPC_SRC ${BASE_RPC_SRC} dllmain.cpp)
    endif(WIN32)
else(${BUILD_DYNAMIC_LIB})
    set(RPC_LIBRARY_TYPE STATIC)
endif(${BUILD_DYNAMIC_LIB})

if(WIN32)
    add_definitions(-DDISCORD_WINDOWS)
    set(BASE_RPC_SRC ${BASE_RPC_SRC} connection_win.cpp discord_register_win.cpp)
    add_library(discord-rpc ${RPC_LIBRARY_TYPE} ${BASE_RPC_SRC})
    target_compile_options(discord-rpc PRIVATE /EHsc
        /MT
        /Wall
        /wd4100 # unreferenced formal parameter
        /wd4514 # unreferenced inline
        /wd4625 # copy constructor deleted
        /wd5026 # move constructor deleted
        /wd4626 # move assignment operator deleted
        /wd4668 # not defined preprocessor macro
        /wd4710 # function not inlined
        /wd4711 # function was inlined
        /wd4820 # structure padding
        /wd4946 # reinterpret_cast used between related classes
        /wd5027 # move assignment operator was implicitly defined as deleted
    )
endif(WIN32)

if(UNIX)
    set(BASE_RPC_SRC ${BASE_RPC_SRC} connection_unix.cpp)

    if (APPLE)
        add_definitions(-DDISCORD_OSX)
        set(BASE_RPC_SRC ${BASE_RPC_SRC} discord_register_osx.m)
    else (APPLE)
        add_definitions(-DDISCORD_LINUX)
        set(BASE_RPC_SRC ${BASE_RPC_SRC} discord_register_linux.cpp)
    endif(APPLE)

    add_library(discord-rpc ${RPC_LIBRARY_TYPE} ${BASE_RPC_SRC})
    target_link_libraries(discord-rpc PUBLIC pthread)
    target_compile_options(discord-rpc PRIVATE
        -g
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wno-unknown-pragmas # pragma push thing doesn't work on clang
        -Wno-old-style-cast # it's fine
        -Wno-c++98-compat # that was almost 2 decades ago
        -Wno-c++98-compat-pedantic
        -Wno-missing-noreturn
        -Wno-padded # structure padding
        -Wno-covered-switch-default
        -Wno-exit-time-destructors # not sure about these
        -Wno-global-constructors
    )
    target_compile_options(discord-rpc PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-std=c++14>)

    if (APPLE)
        target_link_libraries(discord-rpc PRIVATE "-framework AppKit")
    endif (APPLE)
endif(UNIX)

target_include_directories(discord-rpc PRIVATE ${RAPIDJSON}/include)

if (NOT ${ENABLE_IO_THREAD})
    add_definitions(discord-rpc PUBLIC -DDISCORD_DISABLE_IO_THREAD)
endif (NOT ${ENABLE_IO_THREAD})

if (${BUILD_DYNAMIC_LIB})
    target_compile_definitions(discord-rpc PUBLIC -DDISCORD_DYNAMIC_LIB)
    target_compile_definitions(discord-rpc PRIVATE -DDISCORD_BUILDING_SDK)
endif(${BUILD_DYNAMIC_LIB})

add_dependencies(discord-rpc clangformat)

# install

install(
    TARGETS discord-rpc
    EXPORT "discord-rpc"
    RUNTIME
        DESTINATION "bin"
        CONFIGURATIONS Release
    LIBRARY
        DESTINATION "lib"
        CONFIGURATIONS Release
    ARCHIVE
        DESTINATION "lib"
        CONFIGURATIONS Release
    INCLUDES
        DESTINATION "include"
)

install(
    FILES
        "../include/discord-rpc.h"
    DESTINATION "include"
)
