add_library(nspsl STATIC "")
file(GLOB_RECURSE SRC "src/*.c")
target_sources(nspsl PRIVATE ${SRC})
target_include_directories(nspsl PUBLIC include PRIVATE src)
