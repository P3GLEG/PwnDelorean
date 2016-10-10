project(pwndelorean)
cmake_minimum_required(VERSION 3.1)
include(CheckCXXCompilerFlag)
enable_testing()

set(THREADS_PREFER_PTHREAD_FLAG ON)
set(LIBGIT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../deps/libgit2)
set(LIBGIT_INCLUDE_DIR ${LIBGIT_SOURCE_DIR}/../include)
set(PLOG_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../deps/plog/include)
set(RE2_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../deps/re2/re2)
set(GTEST_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../deps/googletest/build/googlemock/gtest)
set(GTEST_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../deps/googletest/googletest/include)

#message("Include: ${GTEST_INCLUDE_DIR}")

file(GLOB_RECURSE PWNDELOREAN_HEADERS ${CMAKE_SOURCE_DIR}/../src)
file(GLOB_RECURSE PWNDELOREAN_SOURCES ${CMAKE_SOURCE_DIR}/../src/*.cpp)


find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

OPTION (BUILD_SHARED_LIBS "Build Shared Library (OFF for Static)" OFF)
OPTION (BUILD_TESTS "Build Tests" ON)

FILE(COPY ${CMAKE_SOURCE_DIR}/../patterns DESTINATION .)

INCLUDE_DIRECTORIES(${PLOG_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${RE2_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${LIBGIT_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${PWNDELOREAN_HEADERS})
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR})

add_executable(runTests all_tests.cpp gitengine_tests.cpp engine_tests.cpp ../src/engine.cpp ../src/gitengine.cpp )
target_link_libraries(runTests ${GTEST_SOURCE_DIR}/libgtest.a stdc++fs re2 ${OPENSSL_LIBRARIES} Threads::Threads ${LIBGIT_SOURCE_DIR}/build/libgit2.a ssh2 z http_parser)

#ADD_EXECUTABLE(pwndelorean ${PWNDELOREAN_SOURCES})
#All these libraries are required by libgit2
#TARGET_LINK_LIBRARIES(pwndelorean  ${GTEST_SOURCE_DIR}/build/libgtest.a)
