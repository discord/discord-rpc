# Discord RPC

This is a lib and a couple of quick demos, one that implements the very minimal subset to show
current status, and one that is more complete. The idea here is to give you an lib that implements
the rpc connection and wraps sending events, and a basic example that uses it; you can use the lib
directly if you like, or use it as a guide to writing your own if it doesn't suit your game as is.

PRs/feedback welcome if you have an improvement everyone might want.

## Usage

There's a CMake file that should be able to generate the lib for you; I use it like this:
```sh
    cd /path/to/discord-rpc
    mkdir build
    cd build
    cmake ..
    cmake --build . --config Release
```
Sometimes I use the generated project files.

## Sample: send-presence

This is a text adventure "game" that inits/deinits the connection to Discord, and sends a presence
update on each command.
