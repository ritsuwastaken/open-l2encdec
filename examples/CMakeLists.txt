cmake_minimum_required(VERSION 3.14)
project(examples)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

if(NOT L2ENCDEC_DIR)
    set(L2ENCDEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
    add_subdirectory(${L2ENCDEC_DIR} ${CMAKE_CURRENT_BINARY_DIR}/l2encdec)
endif()

add_subdirectory(cli)
add_subdirectory(txt211json)
if(NOT MINGW)
    add_subdirectory(utx121webp)
endif()
