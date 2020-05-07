cmake_minimum_required(VERSION 3.8)

project(WPDViewer VERSION 1.0 LANGUAGES CXX)

set(target WPD)

option(BUILD_SHARED_LIBS "Build libs as shared libraries." OFF)
option(BUILD_EXAMPLE "Build example." OFF)
option(BUILD_TESTING "Build all tests." OFF)

if (NOT BUILD_SHARED_LIBS)
    set(BUILD_STATIC_LIBS ON)
else ()
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON) # Little hack
endif ()

#------------------------------------------------------
# Lib
#------------------------------------------------------
set(sources src/WPDEnumerator.cpp
        src/WPDObjectIterator.cpp
        src/WPDException.cpp
        src/WPDObject.cpp
        src/WPDUtils.cpp
        src/WPDDevice.cpp)

add_library(${target} ${sources})
target_compile_features(${target} PRIVATE cxx_std_11)
target_include_directories(${target} PUBLIC include)
target_link_libraries(${target} PortableDeviceGuids.lib)

#include(GenerateExportHeader)
#GENERATE_EXPORT_HEADER(${target}
#        BASE_NAME ${target}
#        EXPORT_MACRO_NAME ${target}_API
#        EXPORT_FILE_NAME ${target}.h
#        STATIC_DEFINE ${target}_STATIC
#        )


#------------------------------------------------------
# Examples
#------------------------------------------------------
if (BUILD_EXAMPLE)
    add_subdirectory(examples)
endif (BUILD_EXAMPLE)

#------------------------------------------------------
# Tests
#------------------------------------------------------
if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif (BUILD_TESTING)