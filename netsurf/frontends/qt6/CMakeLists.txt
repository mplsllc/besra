find_package(Qt6 COMPONENTS Core Gui Widgets PrintSupport REQUIRED)

set(CMAKE_AUTOMOC ON)

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
    browsertab.cpp
    mainwindow.cpp
    dialogs.cpp
    corewindow.cpp
    panels.cpp
    preferences.cpp
    downloads.cpp
    ${BESRA_QT6_QRC_SOURCES}
)

target_link_libraries(netsurf-qt6 PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::PrintSupport
    netsurf
)

if(WIN32)
    target_link_libraries(netsurf-qt6 PRIVATE ws2_32)
endif()

target_include_directories(netsurf-qt6 PRIVATE
    ${CMAKE_SOURCE_DIR}/netsurf
)
