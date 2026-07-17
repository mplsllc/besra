add_library(utf8proc STATIC "")
target_sources(utf8proc PRIVATE src/utf8proc.c)
target_include_directories(utf8proc PUBLIC include/libutf8proc PRIVATE src)
