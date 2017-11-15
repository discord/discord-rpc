# Discord RPC

This is a lib and quick demos that implement the very minimal subset to show current status, and
have callbacks for where a more complete game would do more things. You can use the lib directly
if you like, or use it as a guide to writing your own if it doesn't suit your game as is.

PRs/feedback welcome if you have an improvement everyone might want, or can describe how this
doesn't meet your needs.

## Documentation

The most up to date documentation for Rich Presence can always be found in our [developer site](https://discordapp.com/developers/docs/topics/rich-presence)!

## Usage

Zeroith, you should be set up to build things because you are a game developer, right?

First, head on over to the [Discord developers site](https://discordapp.com/developers/applications/me)
and make yourself an app. Keep track of `Client ID` -- you'll need it here to pass to the init
function.

### From package

Download a release package, extract it, add `/include` to your compile includes, `/lib` to your
linker paths, and link with `discord-rpc`.

Note that the release packages were compiled using Visual Studio 2015, so the [Visual C++ Redistributable for VS2015](https://www.microsoft.com/en-us/download/details.aspx?id=48145) will be a requirement for your game. If you wish to avoid this dependency, you should compile the libraries yourself using whatever dependencies are already in your game.

### From repo

There's a [CMake](https://cmake.org/download/) file that should be able to generate the lib for
you; I use it like this:
```sh
    cd <path to discord-rpc>
    mkdir build
    cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=<path to install discord-rpc to>
    cmake --build . --config Release --target install
```
Sometimes I use the generated project files. There are a couple of CMake options you might care about:

| flag | default | does |
|------|---------|------|
| `ENABLE_IO_THREAD` | `ON` | When enabled, we start up a thread to do io processing, if disabled you should call `Discord_UpdateConnection` yourself.
| [`BUILD_SHARED_LIBS`](https://cmake.org/cmake/help/v3.7/variable/BUILD_SHARED_LIBS.html) | `OFF` | Build library as a DLL

You can also try the `build.py` script for an easy compile for whatever OS you are on.

## Sample: send-presence

This is a text adventure "game" that inits/deinits the connection to Discord, and sends a presence
update on each command.

## Sample: button-clicker

This is a sample [Unity](https://unity3d.com/) project that wraps a DLL version of the library, and
sends presence updates when you click on a button.

## Sample: unrealstatus

This is a sample [Unreal](https://www.unrealengine.com) project that wraps the DLL version of the
library with an Unreal plugin, exposes a blueprint class for interacting with it, and uses that to
make a very simple UI.
