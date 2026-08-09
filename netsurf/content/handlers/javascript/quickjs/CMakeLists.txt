add_library(js-bindings STATIC
    quickjs_backend.c
)

target_include_directories(js-bindings PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../
    ${CMAKE_SOURCE_DIR}/libquickjs
)

target_link_libraries(js-bindings PUBLIC qjs dom wapcaplet nslog nsutils css netsurf)
