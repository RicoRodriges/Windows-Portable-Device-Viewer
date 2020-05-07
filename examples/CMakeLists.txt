cmake_minimum_required(VERSION 3.8)

add_executable(enumerate_devices enumerate_devices.cpp)
target_link_libraries(enumerate_devices ${target})

add_executable(enumerate_files enumerate_files.cpp)
target_link_libraries(enumerate_files ${target})

add_executable(file_operations file_operations.cpp)
target_link_libraries(file_operations ${target})