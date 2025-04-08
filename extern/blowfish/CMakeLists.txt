cmake_minimum_required(VERSION 3.10)
project(blowfish VERSION 1.0.0 LANGUAGES CXX)
add_library(blowfish STATIC blowfish.cpp)
target_include_directories(blowfish PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(blowfish PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)
