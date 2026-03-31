cmake_minimum_required(VERSION 3.16)

project(Monastery VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets PrintSupport)

qt_standard_project_setup()

qt_add_executable(Monastery
    main.cpp
    MonasteryFrame.cpp
    MonasteryEditor.cpp
)

target_sources(Monastery PRIVATE resources.qrc)

target_link_libraries(Monastery PRIVATE Qt6::Core Qt6::Widgets Qt6::PrintSupport)

set_target_properties(Monastery PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)