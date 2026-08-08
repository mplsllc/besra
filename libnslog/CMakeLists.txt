find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(nslog_parser src/filter-parser.y ${CMAKE_CURRENT_BINARY_DIR}/filter-parser.c DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/filter-parser.h)
FLEX_TARGET(nslog_lexer src/filter-lexer.l ${CMAKE_CURRENT_BINARY_DIR}/filter-lexer.c DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/filter-lexer.h)
ADD_FLEX_BISON_DEPENDENCY(nslog_lexer nslog_parser)

add_library(nslog STATIC
    src/core.c
    src/filter.c
    ${BISON_nslog_parser_OUTPUTS}
    ${FLEX_nslog_lexer_OUTPUTS}
    ${CMAKE_CURRENT_BINARY_DIR}/filter-lexer.h
    ${CMAKE_CURRENT_BINARY_DIR}/filter-parser.h
)

target_include_directories(nslog PUBLIC include PRIVATE src ${CMAKE_CURRENT_BINARY_DIR})
# Flex generates some warnings
set_source_files_properties(${FLEX_nslog_lexer_OUTPUTS} PROPERTIES COMPILE_FLAGS "-Wno-unused-parameter -Wno-sign-compare")
