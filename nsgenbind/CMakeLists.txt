find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

# nsgenbind lexer and parser
BISON_TARGET(nsgenbind_parser src/nsgenbind-parser.y ${CMAKE_CURRENT_BINARY_DIR}/nsgenbind-parser.c COMPILE_FLAGS "-t -p nsgenbind_" DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/nsgenbind-parser.h)
FLEX_TARGET(nsgenbind_lexer src/nsgenbind-lexer.l ${CMAKE_CURRENT_BINARY_DIR}/nsgenbind-lexer.c COMPILE_FLAGS "-P nsgenbind_" DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/nsgenbind-lexer.h)
ADD_FLEX_BISON_DEPENDENCY(nsgenbind_lexer nsgenbind_parser)

# webidl lexer and parser
BISON_TARGET(webidl_parser src/webidl-parser.y ${CMAKE_CURRENT_BINARY_DIR}/webidl-parser.c COMPILE_FLAGS "-t -p webidl_" DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/webidl-parser.h)
FLEX_TARGET(webidl_lexer src/webidl-lexer.l ${CMAKE_CURRENT_BINARY_DIR}/webidl-lexer.c COMPILE_FLAGS "-P webidl_" DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/webidl-lexer.h)
ADD_FLEX_BISON_DEPENDENCY(webidl_lexer webidl_parser)

add_executable(nsgenbind
    src/nsgenbind.c
    src/utils.c
    src/output.c
    src/webidl-ast.c
    src/nsgenbind-ast.c
    src/ir.c
    src/duk-libdom.c
    src/duk-libdom-interface.c
    src/duk-libdom-dictionary.c
    src/duk-libdom-common.c
    src/duk-libdom-generated.c
    ${BISON_nsgenbind_parser_OUTPUTS}
    ${FLEX_nsgenbind_lexer_OUTPUTS}
    ${BISON_webidl_parser_OUTPUTS}
    ${FLEX_webidl_lexer_OUTPUTS}
    ${CMAKE_CURRENT_BINARY_DIR}/nsgenbind-parser.h
    ${CMAKE_CURRENT_BINARY_DIR}/nsgenbind-lexer.h
    ${CMAKE_CURRENT_BINARY_DIR}/webidl-parser.h
    ${CMAKE_CURRENT_BINARY_DIR}/webidl-lexer.h
)

target_include_directories(nsgenbind PRIVATE src ${CMAKE_CURRENT_BINARY_DIR} include)
set_source_files_properties(${FLEX_nsgenbind_lexer_OUTPUTS} ${FLEX_webidl_lexer_OUTPUTS} PROPERTIES COMPILE_FLAGS "-Wno-unused-parameter -Wno-sign-compare")
