cmake_minimum_required(VERSION 3.8)

set(test_sources main.cpp
        WPDEnumerator_test.cpp
        WPDDevice_test.cpp
        utils.cpp
        )

add_executable(tests ${test_sources})
target_link_libraries(tests ${target} Propsys)
target_include_directories(tests PUBLIC thirdparty/catch2-2.12.1 thirdparty/fakeit-2.0.5)

add_test(all_tests tests)