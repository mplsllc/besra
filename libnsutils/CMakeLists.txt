add_library(nsutils STATIC "")
file(GLOB_RECURSE SRC "src/*.c")
target_sources(nsutils PRIVATE ${SRC})
target_include_directories(nsutils PUBLIC include PRIVATE src)
