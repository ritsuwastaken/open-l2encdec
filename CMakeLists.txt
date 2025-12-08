cmake_minimum_required(VERSION 3.14)
project(l2encdec VERSION 1.3.2 LANGUAGES CXX)

include(FetchContent)
include(GenerateExportHeader)
include(CTest)

FetchContent_Declare(
    mbedtls
    GIT_REPOSITORY https://github.com/Mbed-TLS/mbedtls.git
    GIT_TAG v3.6.5
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    miniz
    GIT_REPOSITORY https://github.com/richgel999/miniz.git
    GIT_TAG 3.1.0
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    blowfish
    GIT_REPOSITORY https://github.com/avinal/blowfish.git
    GIT_TAG 54482712989ef129929b9ba375a404bf2707a477
)

set(MBEDTLS_FATAL_WARNINGS OFF CACHE BOOL "" FORCE)
set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(ENABLE_PROGRAMS OFF CACHE BOOL "" FORCE)
set(MBEDTLS_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/configs/mbedtls_config.h" CACHE STRING "" FORCE)
set(DISABLE_PACKAGE_CONFIG_AND_INSTALL ON CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(mbedtls miniz blowfish)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(L2ENCDEC_BUILD_TESTS "Build unit tests" OFF)
option(L2ENCDEC_BUILD_EXAMPLES "Build examples" OFF)

add_library(${PROJECT_NAME}
    src/l2encdec.cpp
    src/blowfish.cpp
    src/rsa.cpp
    src/utils.cpp
    src/xor_utils.cpp
    src/zlib_utils.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

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
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE mbedcrypto miniz blowfish)

if(L2ENCDEC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(L2ENCDEC_BUILD_EXAMPLES)
    set(L2ENCDEC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(examples)
endif()
