add_library(nsgif STATIC "")
set(SRC
    src/gif.c
    src/lzw.c
)
target_sources(nsgif PRIVATE ${SRC})
target_include_directories(nsgif PUBLIC include PRIVATE src)
