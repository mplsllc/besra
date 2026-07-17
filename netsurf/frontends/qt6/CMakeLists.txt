find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_executable(netsurf-qt6
    main.cpp
    stubs.cpp
    schedule.cpp
    layout.cpp
    plotters.cpp
    window.cpp
)

target_link_libraries(netsurf-qt6 PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    netsurf
)

target_include_directories(netsurf-qt6 PRIVATE
    ${CMAKE_SOURCE_DIR}/netsurf
)
