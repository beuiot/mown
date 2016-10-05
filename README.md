# Mown - Static website publishing at peace
> My Own Website Now!


## Purpose

Mown aims to be a sustainable and comfortable web content authoring tool:
- Simplest server-side setup (no scripts, no database)
- Easy to setup authoring environment (do not depend on developer environments or tools)
- Instant preview during content authoring
- Little to no configuration

## Features

- Articles and standalone pages
- RSS feed
- Instant preview generated automatically when source files are modified
- Comment snippets support
- Open source (MIT)
- Binaries available (for now only Windows 64bits)

## Demo video

[![IMAGE ALT TEXT HERE](http://img.youtube.com/vi/iByF3Gdrh10/0.jpg)](http://www.youtube.com/watch?v=iByF3Gdrh10)

## Contributing

Issues, comments and external pull requests are welcome; even more if they fit the project purpose mentioned above!

## Internals

### Design goals

- C++/boost library
- Qt/Webkit UI
- YAML files
- simple template format
- auto generation of every configuration and template files

### Build guide

Mown depends on the following libraries and tools:

- Qt 5.7
- Boost 1.57.0 or later
- CMake 2.8.11 or later
- yaml-cpp

##### Windows (MSVC 14 x64)

###### Install requirements

- Install latest cmake
- Install QT 5.7 (msvc 14 2015 x64 binaries)
- Install Boost MSVC 14 2015 64bits windows pre-built binaries

###### Environment/CMake variables

- BOOST_ROOT to the boost dir (for instance C:\local\boost_1_61_0)
- CMAKE_PREFIX_PATH to the Qt build dir (for instance C:\Qt\5.7\msvc2015_64)

###### Build yaml-cpp

```sh
(in mown repository root)
$ git submodule update --init
$ cd dependencies\yaml-cpp
$ mkdir build && cd build
$ cmake -G "Visual Studio 14 2015 Win64" ..
$ cmake --build . --config Debug
$ cmake --build . --config Release
```

###### Build Mown

```sh
$ (in mown repository root)
$ mkdir build && cd build
$ cmake -G "Visual Studio 14 2015 Win64" ..
$ cmake --build .
```
