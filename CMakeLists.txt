cmake_minimum_required(VERSION 3.0.0)

project(TimeRecorderCore VERSION 0.1.0)

set(PROJECT_SOURCES
        "src/main.cpp"
        "src/tr_core.cpp"
        "src/tr_core.h"
        
        "lib/sqlite3.c"
        "lib/sqlite3.h"
)

include_directories(lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin) 
add_executable(TimeRecorderCore ${PROJECT_SOURCES})
