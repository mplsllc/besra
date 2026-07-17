cmake_minimum_required(VERSION 3.10)
project(Besra C CXX)

option(BESRA_BUILD_TESTS "Build tests" OFF)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L")

# Include modules
include(FindPkgConfig)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

option(BESRA_BUILD_TESTS "Build libcss and libdom check-based tests" OFF)

if(BESRA_BUILD_TESTS)
    enable_testing()
endif()

# Submodules
add_compile_options(
    -g -O0 -Wall -Wextra -Wno-unused-parameter
    "-DSTMTEXPR=1"
)

add_subdirectory(libwapcaplet)
add_subdirectory(libparserutils)
add_subdirectory(libhubbub)
add_subdirectory(libdom)
add_subdirectory(libcss)
add_subdirectory(libnsgif)
add_subdirectory(libnsbmp)
add_subdirectory(libnsutils)
add_subdirectory(libnspsl)
add_subdirectory(libsvgtiny)
add_subdirectory(libutf8proc)
add_subdirectory(libnslog)
add_subdirectory(nsgenbind)

# Netsurf core and frontend
add_subdirectory(netsurf)
