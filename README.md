# Windows portable device viewer
[![Build status](https://ci.appveyor.com/api/projects/status/nquknw2kly0ayqul/branch/master?svg=true)](https://ci.appveyor.com/project/RicoRodriges/windows-portable-device-viewer/branch/master)
[![codecov](https://codecov.io/gh/RicoRodriges/Windows-Portable-Device-Viewer/branch/master/graph/badge.svg)](https://codecov.io/gh/RicoRodriges/Windows-Portable-Device-Viewer)

This set of classes allows viewing files on portable devices such as IPhones, digital cameras and other windows portable devices (WPD). Currently implemented in read-only mode.

Available classes:
* `WPDEnumerator` allows to get information about all connected portable devices.
* `WPDDevice` interacts with a particular devices and its file system.
* `WPDObject` is an object (file, directory or storage).
* `WPDObjectIterator` displays all files in a specific directory.
* `WPDException` is runtime exception with HRESULT code.

### How to use
You can find examples in [examples](examples) directory.

Use in cmake project:
```cmake
# Compile
option(BUILD_SHARED_LIBS "" OFF)
add_subdirectory(wpd) # Directory with cloned repository

# Link
target_link_libraries(my_target_name WPD)
```

Here is some tips:
* This library is not thread-safe and will never be because it may produce "object in use" errors by the Microsoft API design. Do not use `WPDDevice` instance in different threads at the same time.
* `WPDDevice` is a general class. Another classes have references. Do not destroy `WPDDevice` object if you're going to continue working with device's objects.
* If you need some information about object you could use `WPDObject` instance or `WPDDevice` methods:
  * `WPDObject` is lazy, has cache and uses a single call to get all information first time. Use it to get all information.
  * `WPDDevice` uses several calls but always gives fresh information. Use it to get only a small set of object attributes.

## Build

### Prerequisites
* Visual Studio or another C++11 compiler
* [CMake](https://cmake.org/) 3.8+

### Generation of project
Generate cmake project like this: 
```cmd
mkdir build
cd build
cmake -DPARAM1=OFF -DPARAM2=ON -G "cmake generator" -A "cmake platform"
```

Available parameters:
* `BUILD_SHARED_LIBS ON/OFF` - compile as shared or static library.
* `BUILD_EXAMPLE ON/OFF` - compile examples from [examples](examples) directory.
* `BUILD_TESTS ON/OFF` - compile unit tests.

Generate project as 64-bit static library for Visual Studio 2017:
```cmd
cmake -DBUILD_SHARED_LIBS=OFF -G "Visual Studio 15 2017" -A "x64"
```

### Compile
```cmd
cmake --build . --config "Release"
```
where `--config` could be `Debug`, `Release`

### Running tests
Tests are powered by [Catch2](https://github.com/catchorg/Catch2) and [FakeIt](https://github.com/eranpeer/FakeIt).

The project must be compiled as a static library with `Debug` config and `BUILD_TESTS=ON` parameter.
```cmd
cmake -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTS=ON
cmake --build . --config "Debug"
```

After all build steps unit tests can be executed
```cmd
cd build
ctest
```
or without `ctest`:
```cmd
cd build/Debug
tests.exe
```