cmake_minimum_required(VERSION 3.10)
project(getopt VERSION 1.0.0 LANGUAGES C)
add_library(getopt STATIC getopt.c)
target_include_directories(getopt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(getopt PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)
