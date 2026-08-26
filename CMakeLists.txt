cmake_minimum_required(VERSION 4.3)
project(SerialForge)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        SerialPort
        REQUIRED)
set(CMAKE_AUTOUIC_SEARCH_PATHS
        "${CMAKE_CURRENT_SOURCE_DIR}/ui"
)

add_executable(serialforge src/main.cpp
        src/mainwindow.cpp
        inc/mainwindow.h
        ui/mainwindow.ui
        src/common.cpp
        inc/common.h
        src/SerialConnection.cpp
        inc/SerialConnection.h
        src/SerialController.cpp
        inc/SerialController.h
)
target_link_libraries(serialforge
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::SerialPort
)

target_include_directories(serialforge PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        inc
)