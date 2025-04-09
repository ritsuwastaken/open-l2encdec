cmake_minimum_required(VERSION 3.28)
project(l2encdec VERSION 1.1.11 LANGUAGES CXX)

include(FetchContent)
include(GenerateExportHeader)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

add_library(${PROJECT_NAME}
    src/l2encdec.cpp
    src/rsa.cpp
    src/xor.cpp
    src/bf.cpp
    src/zlib_utils.cpp
)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

generate_export_header(${PROJECT_NAME}
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/include/l2encdec-export.h
    EXPORT_MACRO_NAME L2ENCDEC_API
)

if(BUILD_SHARED_LIBS)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
)


FetchContent_Declare(
    mbedtls
    GIT_REPOSITORY https://github.com/Mbed-TLS/mbedtls.git
    GIT_TAG v3.6.3
)
set(MBEDTLS_FATAL_WARNINGS OFF CACHE BOOL "" FORCE)
set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(ENABLE_PROGRAMS OFF CACHE BOOL "" FORCE)
set(MBEDTLS_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake/configs/mbedtls_config.h" CACHE STRING "" FORCE)
set(DISABLE_PACKAGE_CONFIG_AND_INSTALL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(mbedtls)

FetchContent_Declare(
    miniz
    GIT_REPOSITORY https://github.com/richgel999/miniz.git
    GIT_TAG ae07a3615925f252f54fecb84aee2e3cfd4f0676
)
FetchContent_MakeAvailable(miniz)

add_subdirectory(extern/blowfish)

target_link_libraries(${PROJECT_NAME} PRIVATE mbedcrypto miniz blowfish)
