cmake_minimum_required(VERSION 3.28)
project(l2encdec VERSION 1.1.0 LANGUAGES CXX C)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS OFF)
set(MBEDTLS_FATAL_WARNINGS OFF CACHE BOOL "" FORCE)
set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(ENABLE_PROGRAMS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    mbedtls
    GIT_REPOSITORY https://github.com/Mbed-TLS/mbedtls.git
    GIT_TAG        v3.6.3
)
FetchContent_MakeAvailable(mbedtls)

FetchContent_Declare(
    miniz
    GIT_REPOSITORY https://github.com/richgel999/miniz.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(miniz)

set(SOURCES
    src/l2encdec.cpp
    src/rsa.cpp
    src/xor.cpp
    src/bf.cpp
    src/zlib_utils.cpp
    third_party/blowfish/blowfish.cpp
)

set(INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    third_party/blowfish
)

set(EXE_SOURCES src/main.cpp)
set(EXE_INCLUDES "")
if(MSVC)
    list(APPEND EXE_SOURCES third_party/getopt/getopt.c)
    list(APPEND EXE_INCLUDES third_party/getopt)
endif()

add_library(${PROJECT_NAME} ${SOURCES})
target_compile_definitions(${PROJECT_NAME} PUBLIC L2ENCDEC_STATIC)
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDES})
target_link_libraries(${PROJECT_NAME} PRIVATE mbedtls mbedcrypto mbedx509 miniz)

add_executable(${PROJECT_NAME}_exe ${EXE_SOURCES} ${SOURCES})
target_compile_definitions(${PROJECT_NAME}_exe PUBLIC L2ENCDEC_STATIC)
target_include_directories(${PROJECT_NAME}_exe PRIVATE ${EXE_INCLUDES} ${INCLUDES})
target_link_libraries(${PROJECT_NAME}_exe PRIVATE ${PROJECT_NAME} mbedtls mbedcrypto mbedx509 miniz)
target_compile_definitions(${PROJECT_NAME}_exe PRIVATE L2ENCDEC_STATIC)
set_target_properties(${PROJECT_NAME}_exe PROPERTIES 
    OUTPUT_NAME "${PROJECT_NAME}$<$<PLATFORM_ID:Darwin>:_osx>$<$<PLATFORM_ID:Windows>:_win>"
)

if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE -static -lwinpthread)
    target_link_options(${PROJECT_NAME}_exe PRIVATE -static -lwinpthread)
endif()
