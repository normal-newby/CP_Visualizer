cmake_minimum_required(VERSION 3.19)
project(CP_Visualizer LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql Test)
qt_standard_project_setup()

qt_add_library(cp_core STATIC
    src/components/graph.h src/components/graph.cpp
    src/components/node.h src/components/node.cpp
    src/components/edge.h src/components/edge.cpp
)
target_include_directories(cp_core PUBLIC src)
target_link_libraries(cp_core PUBLIC Qt::Core)

qt_add_library(cp_persistence STATIC
    src/persistence/database.h
    src/persistence/database.cpp
    src/persistence/graph-repository.h
    src/persistence/graph-repository.cpp
)
target_link_libraries(cp_persistence PUBLIC cp_core Qt::Sql)

qt_add_resources(cp_persistence "schema"
    PREFIX "/"
    BASE src/persistence
    FILES src/persistence/schema.sql)

qt_add_executable(CP_Visualizer
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/main-window.cpp
    src/main-window.h
    src/main-window.ui
    src/persistence/schema.sql
)

target_link_libraries(CP_Visualizer
    PRIVATE
        cp_core
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
    qt_add_executable(${name}-test src/tests/${name}-test.cpp)
    target_link_libraries(${name}-test PRIVATE cp_core cp_persistence Qt::Test)
    add_test(NAME ${name}-test COMMAND ${name}-test)
endfunction()

cp_add_test(graph)
cp_add_test(graph-repository)
