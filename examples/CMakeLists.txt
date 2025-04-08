cmake_minimum_required(VERSION 3.28)
project(examples)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

set(L2ENCDEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_subdirectory(${L2ENCDEC_DIR} ${CMAKE_CURRENT_BINARY_DIR}/l2encdec)
add_subdirectory(txt211json)
add_subdirectory(utx121webp)
