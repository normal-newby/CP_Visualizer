cmake_minimum_required(VERSION 3.19)
project(CP_Visualizer LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql Test)

qt_standard_project_setup()

qt_add_executable(CP_Visualizer
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui

    #Components
    src/Components/Graph.h
    src/Components/Node.h
    src/Components/Edge.h
    src/Components/Edge.cpp
    src/Components/Node.cpp
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

# Tests

enable_testing()

function(cp_add_test name)
    qt_add_executable(${name}_test src/Tests/${name}_test.cpp)
    target_include_directories(${name}_test PRIVATE src)
    target_link_libraries(${name}_test PRIVATE Qt::Core Qt::Test)
    add_test(NAME ${name}_test COMMAND ${name}_test)
endfunction()

cp_add_test(graph)
