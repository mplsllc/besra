find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

qt_add_resources(BESRA_QT6_QRC_SOURCES res/besra.qrc)

add_executable(netsurf-qt6
    main.cpp
    stubs.cpp
    schedule.cpp
    layout.cpp
    plotters.cpp
    window.cpp
    bitmap.cpp
    resources.cpp
    ${BESRA_QT6_QRC_SOURCES}
)

target_link_libraries(netsurf-qt6 PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    netsurf
)

if(WIN32)
    target_link_libraries(netsurf-qt6 PRIVATE ws2_32)
endif()

target_include_directories(netsurf-qt6 PRIVATE
    ${CMAKE_SOURCE_DIR}/netsurf
)
