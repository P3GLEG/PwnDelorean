project(pwndelorean)
cmake_minimum_required(VERSION 3.1)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)
set(LIBGIT_DIR ${CMAKE_SOURCE_DIR}/libgit2)
find_library(${LIBGIT_DIR} libgit2)


OPTION (BUILD_SHARED_LIBS "Build Shared Library (OFF for Static)" OFF)
OPTION (BUILD_TESTS "Build Tests" ON)

# Compile and link LIBGIT2
INCLUDE_DIRECTORIES(${LIBGIT2_INCLUDE_DIRS})
ADD_EXECUTABLE(pwndelorean main.c)
TARGET_LINK_LIBRARIES(pwndelorean  ${OPENSSL_LIBRARIES}  Threads::Threads  ${LIBGIT_DIR}/build/libgit2.a ssh2 z http_parser)
