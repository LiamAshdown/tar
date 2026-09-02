cmake_minimum_required(VERSION 4.1)
project(tar)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        zlib
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG        v1.3.1
        OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
        cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
        GIT_TAG        v3.3.1
)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.15.2
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(zlib cxxopts googletest)

if(TARGET zlibstatic AND NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
elseif(TARGET zlib AND NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS zlib)
endif()

add_executable(tar main.cpp
        runner.h
        tar_header.h
        file.h
        util.h
)
target_include_directories(tar PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(tar PRIVATE -Wall -Wextra)
target_link_libraries(tar PRIVATE ZLIB::ZLIB cxxopts::cxxopts)

enable_testing()

add_executable(tar_tests
        tests/util_tests.cpp
        tests/tar_header_tests.cpp
        tests/runner_tests.cpp
        tests/file_tests.cpp
        tests/test_helpers.h
)
target_include_directories(tar_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(tar_tests PRIVATE -Wall -Wextra)
target_link_libraries(tar_tests PRIVATE ZLIB::ZLIB GTest::gtest_main)

include(GoogleTest)
# A hang in the inflate loop would otherwise stall the build forever.
gtest_discover_tests(tar_tests PROPERTIES TIMEOUT 30)
