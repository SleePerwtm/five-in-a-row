cmake_minimum_required(VERSION 3.19)
project(five-in-a-row VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(five-in-a-row
    WIN32 MACOSX_BUNDLE
    main.cpp
    board.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    chessboardwidget.cpp
    chessboardwidget.h
    game.cpp
    game.h
)

target_include_directories(five-in-a-row
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(five-in-a-row
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS five-in-a-row
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET five-in-a-row
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
