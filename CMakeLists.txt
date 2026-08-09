cmake_minimum_required(VERSION 3.10)
project(Besra VERSION 0.1.2 LANGUAGES C CXX)

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
add_compile_options(-Wall -Wextra -Wno-unused-parameter "-DSTMTEXPR=1")
if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Fast dev-iteration default: unoptimized with full debug info. Release
    # builds (packaging/build-in-debian.sh) pass -DCMAKE_BUILD_TYPE=Release
    # and get CMake's normal optimized flags instead.
    add_compile_options(-g -O0)
endif()

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

set(QJS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(QJS_BUILD_CLI_STATIC OFF CACHE BOOL "" FORCE)
set(QJS_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(libquickjs)

# Netsurf core and frontend
add_subdirectory(netsurf)

include(GNUInstallDirs)
install(FILES packaging/besra.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
)
install(FILES packaging/icons/hicolor/scalable/apps/besra.svg
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
)
foreach(_size 16 22 24 32 48 64 128 256)
    install(FILES packaging/icons/hicolor/${_size}x${_size}/apps/besra.png
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${_size}x${_size}/apps
    )
endforeach()

set(CPACK_PACKAGE_NAME "besra")
set(CPACK_PACKAGE_VENDOR "mplsllc")
set(CPACK_PACKAGE_CONTACT "apt@mp.ls")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A lightweight, independent web engine for the readable modern web")
set(CPACK_PACKAGE_VERSION_MAJOR ${Besra_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${Besra_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${Besra_VERSION_PATCH})

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "apt@mp.ls")
set(CPACK_DEBIAN_PACKAGE_SECTION "web")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/mplsllc/besra")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

# postinst/postrm refresh the icon and desktop-file caches automatically on
# install/upgrade/remove, so `apt install`/`apt upgrade` alone is enough for
# the app icon and menu entry to show up correctly — no manual cache-clear
# commands for the user to run.
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_SOURCE_DIR}/packaging/postinst"
    "${CMAKE_SOURCE_DIR}/packaging/postrm"
)

include(CPack)
