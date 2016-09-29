# Mown

## Requirements

- Qt 5.7
- Boost 1.57.0 or later
- CMake 2.8.11 or later

### Build (MSVC 14 x64)

##### Install requirements

- Install latest cmake
- Install QT 5.7 (msvc 14 2015 x64 binaries)
- Install Boost MSVC 14 2015 64bits windows pre-built binaries

##### Environment variables

- BOOST_ROOT to the boost dir (for instance C:\local\boost_1_61_0)
- CMAKE_PREFIX_PATH to the Qt build dir (for instance C:\Qt\5.7\msvc2015_64)

##### Build yaml-cpp

```sh
(in mown repository root)
$ git submodule update --init
$ cd dependencies\yaml-cpp
$ mkdir build && cd build
$ cmake -G "Visual Studio 14 2015 Win64" ..
$ cmake --build . --config Debug
$ cmake --build . --config Release
```


##### Build Mown

```sh
$ (in mown repository root)
$ mkdir build && cd build
$ cmake -G "Visual Studio 14 2015 Win64" ..
$ cmake --build .
```

##### Deploy qt dlls and required files

```sh
(in build folder)
$ cmake --build . --target windeployqt
```
