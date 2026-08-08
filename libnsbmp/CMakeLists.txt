add_library(nsbmp STATIC "")
set(SRC
    src/libnsbmp.c
)
target_sources(nsbmp PRIVATE ${SRC})
target_include_directories(nsbmp PUBLIC include PRIVATE src)
