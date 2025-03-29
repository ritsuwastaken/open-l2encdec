cmake_minimum_required(VERSION 3.28)
project(l2encdec VERSION 1.0.16 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

set(SOURCES
    src/l2encdec.cpp
    src/rsa.cpp
    src/xor.cpp
    src/bf.cpp
    src/zlib_utils.cpp
    third_party/blowfish/blowfish.cpp
    third_party/miniz/miniz.c
)

set(INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    third_party/blowfish
    third_party/miniz
)

set(EXE_SOURCES src/main.cpp)

set(EXE_INCLUDES "")

if(MSVC)
    list(APPEND EXE_SOURCES third_party/getopt/getopt.c)
    list(APPEND EXE_INCLUDES third_party/getopt)
endif()

if(MSVC)
    find_package(PkgConfig QUIET)
    if (PkgConfig_FOUND)
        pkg_check_modules(GMP IMPORTED_TARGET gmp)
    endif()
else()
    if(MINGW)
        set(GMP_USE_STATIC_LIBS ON)
    endif()
    find_package(GMP QUIET)
    if (GMP_FOUND)
        list(APPEND INCLUDES ${GMP_INCLUDE_DIR})
    endif()
endif()

if(NOT GMP_FOUND)
    list(APPEND SOURCES third_party/mini-gmp/mini-gmp.c)
    list(APPEND INCLUDES third_party/mini-gmp)
endif()

add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDES})
if(BUILD_SHARED_LIBS)
    if(NOT WIN32)
        target_compile_options(${PROJECT_NAME} PRIVATE -fvisibility=hidden)
    endif()
    target_compile_definitions(${PROJECT_NAME} 
        PRIVATE L2ENCDEC_EXPORTS
        PUBLIC L2ENCDEC_SHARED)
endif()

add_executable(${PROJECT_NAME}_exe ${EXE_SOURCES} ${SOURCES})
target_include_directories(${PROJECT_NAME}_exe PRIVATE ${EXE_INCLUDES} ${INCLUDES})
set_target_properties(${PROJECT_NAME}_exe PROPERTIES OUTPUT_NAME "${PROJECT_NAME}$<$<PLATFORM_ID:Darwin>:_osx>")

foreach(TARGET ${PROJECT_NAME} ${PROJECT_NAME}_exe)
    if (GMP_FOUND)
        target_compile_definitions(${TARGET} PRIVATE GMP_FOUND)
        target_link_libraries(${TARGET} PRIVATE $<IF:$<C_COMPILER_ID:MSVC>,PkgConfig::GMP,${GMP_LIB}>)
    endif()
    if (MSVC)
        target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:/MT>)
        target_link_options(${TARGET} PRIVATE /SAFESEH:NO)
    elseif(MINGW)
        target_link_options(${TARGET} PRIVATE -static -lwinpthread)
    endif()
endforeach()
