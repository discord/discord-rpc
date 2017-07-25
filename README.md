# Discord RPC

This is a lib and a couple of quick demos, one that implements the very minimal subset to show
current status, and one that is more complete. The idea here is to give you an lib that implements
the rpc connection and wraps sending events, and a basic example that uses it; you can use the lib
directly if you like, or use it as a guide to writing your own if it doesn't suit your game as is.

PRs/feedback welcome if you have an improvement everyone might want.

## Usage

First, head on over to the [Discord developers site](https://discordapp.com/developers/applications/me)
and make yourself an app. Keep track of `Client ID` -- you'll need it here.

### From package

Download a release package, extract it, add `/include` to your compile includes, `/lib` to your
linker paths, and link with `discord-rpc`.

### From repo

There's a CMake file that should be able to generate the lib for you; I use it like this:
```sh
    cd /path/to/discord-rpc
    mkdir build
    cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/that
    cmake --build . --config Release --target install
```
Sometimes I use the generated project files.

## Sample: send-presence

This is a text adventure "game" that inits/deinits the connection to Discord, and sends a presence
update on each command.
