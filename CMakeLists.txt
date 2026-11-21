cmake_minimum_required(VERSION 3.19)
project(CP_Visualizer LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql)

qt_standard_project_setup()

qt_add_executable(CP_Visualizer
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui

    src/Components/Graph.h
    src/Components/Node.h
    src/Components/Edge.h
)

target_link_libraries(CP_Visualizer
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql
)

install(TARGETS CP_Visualizer
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET CP_Visualizer
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
