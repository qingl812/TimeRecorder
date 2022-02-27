cmake_minimum_required(VERSION 3.5)

# 设置 QT 安装路径
set(CMAKE_PREFIX_PATH "C:/Qt/6.2.2/mingw_64/lib/cmake")

project(TimeRecorder-Qt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
    src/main.cpp
    src/main_window.cpp
    src/main_window.h
)

# SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

include_directories(lib)
add_executable(TimeRecorderQt ${PROJECT_SOURCES})

target_link_libraries(TimeRecorderQt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)