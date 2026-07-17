add_subdirectory(content/handlers/javascript/duktape)

# Host tools
find_package(ZLIB REQUIRED)
add_executable(xxd tools/xxd.c)
add_executable(split-messages tools/split-messages.c)
target_link_libraries(split-messages ZLIB::ZLIB)
target_include_directories(split-messages PRIVATE .)

# Testament
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/testament.h
    COMMAND perl ${CMAKE_CURRENT_SOURCE_DIR}/tools/git-testament.pl ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/testament.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tools/git-testament.pl
)

# Core library
file(GLOB_RECURSE NETSURF_CORE_SOURCES
    content/*.c
    desktop/*.c
    utils/*.c
)

file(GLOB_RECURSE EXCLUDE_DUKTAPE content/handlers/javascript/duktape/*.c)
file(GLOB EXCLUDE_IMAGE_HANDLERS 
    content/handlers/image/nssprite.c 
    content/handlers/image/rsvg.c 
    content/handlers/image/rsvg246.c 
    content/handlers/image/jpegxl.c 
    content/handlers/image/video.c 
    content/handlers/image/webp.c
)
if(EXCLUDE_DUKTAPE)
    list(REMOVE_ITEM NETSURF_CORE_SOURCES ${EXCLUDE_DUKTAPE})
endif()
if(EXCLUDE_IMAGE_HANDLERS)
    list(REMOVE_ITEM NETSURF_CORE_SOURCES ${EXCLUDE_IMAGE_HANDLERS})
endif()

add_library(netsurf STATIC ${NETSURF_CORE_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/testament.h)
target_include_directories(netsurf PUBLIC include ${CMAKE_CURRENT_BINARY_DIR} PRIVATE . frontends)
target_compile_definitions(netsurf PUBLIC 
    "NETSURF_BUILTIN_LOG_FILTER=\"level:WARNING\""
    "NETSURF_BUILTIN_VERBOSE_FILTER=\"level:VERBOSE\""
    WITH_BMP
    WITH_GIF
    WITH_JPEG
    WITH_PNG
    WITH_NS_SVG
)

target_link_libraries(netsurf PUBLIC
    wapcaplet
    parserutils
    hubbub
    dom
    css
    nsgif
    nsbmp
    nsutils
    nspsl
    svgtiny
    utf8proc
    nslog
    duktape
    js-bindings
)

pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(OPENSSL REQUIRED openssl)
pkg_check_modules(PNG REQUIRED libpng)
pkg_check_modules(JPEG REQUIRED libjpeg)

target_link_libraries(netsurf PUBLIC
    ${CURL_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${PNG_LIBRARIES}
    ${JPEG_LIBRARIES}
)
target_include_directories(netsurf PUBLIC
    ${CURL_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    ${JPEG_INCLUDE_DIRS}
)
target_compile_options(netsurf PUBLIC
    ${CURL_CFLAGS}
    ${OPENSSL_CFLAGS}
    ${PNG_CFLAGS}
    ${JPEG_CFLAGS}
)
if(UNIX)
    add_subdirectory(frontends/qt6)
endif()
