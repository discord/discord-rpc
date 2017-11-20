{
  "targets": [
    {
      "target_name": "discord-rpc",
      "target_type": "static_library",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [
        "src/discord-rpc.cpp",
        "src/rpc_connection.cpp",
        "src/serialization.cpp",
        "js/addon.cc",
      ],
      'conditions': [
         ['OS == "win"', {
          'sources': [
            'src/connection_win.cpp',
            'src/discord_register_win.cpp',
            'src/dllmain.cpp'
          ],
        }],
        ['OS == "linux"', {
          'sources': [
            'src/connection_unix.cpp',
            'src/discord_register_linux.cpp',
          ],
        }],
        ['OS == "mac"', {
          'sources': [
            'src/connection_unix.cpp',
            'src/discord_register_osx.m',
          ],
          'defines': ['__MACOSX_CORE__'],
          'link_settings': {
            'libraries': [
              '-framework Foundation'
            ],
          },
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'CLANG_CXX_LIBRARY': 'libc++',
            'OTHER_CFLAGS': [
              '-ObjC++',
              '-std=c++11'
            ],
          },
        }],
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "thirdparty/rapidjson-1.1.0/include",
        "src",
      ],
      'dependencies': [
        "<!(node -p \"require('node-addon-api').gyp\")",
      ],
      "defines": [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
    },
  ]
}
