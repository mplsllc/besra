add_library(wapcaplet STATIC "")
set(SRC
    src/libwapcaplet.c
)
target_sources(wapcaplet PRIVATE ${SRC})
target_include_directories(wapcaplet PUBLIC include PRIVATE src)
