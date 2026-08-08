pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(GTHREAD REQUIRED gthread-2.0)
pkg_check_modules(GMODULE REQUIRED gmodule-2.0)
pkg_check_modules(RSVG librsvg-2.0)

set(LANGUAGES nl de fr it zh_CN)

set(MESSAGE_FILES ${CMAKE_CURRENT_BINARY_DIR}/res/Messages)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/res/Messages
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/res
    COMMAND split-messages -l en -p any -f messages -o ${CMAKE_CURRENT_BINARY_DIR}/res/Messages -z ${CMAKE_SOURCE_DIR}/netsurf/resources/FatMessages
    DEPENDS split-messages ${CMAKE_SOURCE_DIR}/netsurf/resources/FatMessages
)
foreach(LANG IN LISTS LANGUAGES)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/res/${LANG}/Messages
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/res/${LANG}
        COMMAND split-messages -l ${LANG} -p any -f messages -o ${CMAKE_CURRENT_BINARY_DIR}/res/${LANG}/Messages -z ${CMAKE_SOURCE_DIR}/netsurf/resources/FatMessages
        DEPENDS split-messages ${CMAKE_SOURCE_DIR}/netsurf/resources/FatMessages
    )
    list(APPEND MESSAGE_FILES ${CMAKE_CURRENT_BINARY_DIR}/res/${LANG}/Messages)
endforeach()

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/messages_gresource.c
    COMMAND glib-compile-resources --target=${CMAKE_CURRENT_BINARY_DIR}/messages_gresource.c --sourcedir=${CMAKE_CURRENT_BINARY_DIR}/res --generate-source ${CMAKE_CURRENT_SOURCE_DIR}/res/messages.gresource.xml
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res/messages.gresource.xml ${MESSAGE_FILES}
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/netsurf_gresource.c
    COMMAND glib-compile-resources --target=${CMAKE_CURRENT_BINARY_DIR}/netsurf_gresource.c --sourcedir=${CMAKE_CURRENT_SOURCE_DIR}/res --generate-source ${CMAKE_CURRENT_SOURCE_DIR}/res/netsurf.gresource.xml
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res/netsurf.gresource.xml
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ui_gresource.c
    COMMAND glib-compile-resources --target=${CMAKE_CURRENT_BINARY_DIR}/ui_gresource.c --sourcedir=${CMAKE_CURRENT_SOURCE_DIR}/res/gtk3 --sourcedir=${CMAKE_CURRENT_SOURCE_DIR}/res --generate-source ${CMAKE_CURRENT_SOURCE_DIR}/res/ui.gresource.xml
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res/ui.gresource.xml
)

set(FRONTEND_SOURCES
    about.c
    accelerator.c
    bitmap.c
    compat.c
    completion.c
    cookies.c
    corewindow.c
    download.c
    fetch.c
    gdk.c
    gettext.c
    global_history.c
    gui.c
    hotlist.c
    layout_pango.c
    local_history.c
    menu.c
    misc.c
    page_info.c
    plotters.c
    preferences.c
    print.c
    resources.c
    scaffolding.c
    schedule.c
    search.c
    selection.c
    tabs.c
    throbber.c
    toolbar.c
    viewdata.c
    viewsource.c
    window.c
)
list(REMOVE_ITEM FRONTEND_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/sexy_icon_entry.c)

add_executable(netsurf-gtk3
    ${FRONTEND_SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/messages_gresource.c
    ${CMAKE_CURRENT_BINARY_DIR}/netsurf_gresource.c
    ${CMAKE_CURRENT_BINARY_DIR}/ui_gresource.c
)

target_include_directories(netsurf-gtk3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GTK3_INCLUDE_DIRS}
    ${GTHREAD_INCLUDE_DIRS}
    ${GMODULE_INCLUDE_DIRS}
    ${RSVG_INCLUDE_DIRS}
)

target_compile_options(netsurf-gtk3 PRIVATE
    ${GTK3_CFLAGS}
    ${GTHREAD_CFLAGS}
    ${GMODULE_CFLAGS}
    ${RSVG_CFLAGS}
)

target_compile_definitions(netsurf-gtk3 PRIVATE
    "GTK_RESPATH=\"res/\""
    "NETSURF_HOMEPAGE=\"about:welcome\""
    "WITH_GRESOURCE"
)

target_link_libraries(netsurf-gtk3 PRIVATE
    netsurf
    ${GTK3_LIBRARIES}
    ${GTHREAD_LIBRARIES}
    ${GMODULE_LIBRARIES}
    ${RSVG_LIBRARIES}
    m
)
