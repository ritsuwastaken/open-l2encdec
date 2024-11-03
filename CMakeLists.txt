cmake_minimum_required(VERSION 3.28)
project(l2encdec VERSION 1.0.9 LANGUAGES CXX C)

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

if(MSVC)
    list(APPEND SOURCES third_party/getopt/getopt.c)
    list(APPEND INCLUDES third_party/getopt)
endif()

if(MSVC)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GMP IMPORTED_TARGET gmp)
else()
    if(MINGW)
        set(GMP_USE_STATIC_LIBS ON)
    endif()
    find_package(GMP QUIET)
    if (GMP_FOUND)
        list(APPEND INCLUDES ${GMP_INCLUDE_DIR})
    endif()
endif()
if(GMP_FOUND)
    add_compile_definitions(GMP_FOUND)
else()
    list(APPEND SOURCES third_party/mini-gmp/mini-gmp.c)
    list(APPEND INCLUDES third_party/mini-gmp)
endif()

add_library(${PROJECT_NAME} ${SOURCES})
add_executable(${PROJECT_NAME}_exe src/main.cpp ${SOURCES})

if(APPLE)
    set_target_properties(${PROJECT_NAME}_exe PROPERTIES OUTPUT_NAME "${PROJECT_NAME}_osx")
else()
    set_target_properties(${PROJECT_NAME}_exe PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
endif()

foreach(TARGET ${PROJECT_NAME} ${PROJECT_NAME}_exe)
    target_include_directories(${TARGET} PRIVATE ${INCLUDES})
    target_compile_definitions(${TARGET} PRIVATE L2ENCDEC_EXPORTS)

    if(GMP_FOUND AND MSVC)
        target_link_libraries(${TARGET} PRIVATE PkgConfig::GMP)
    elseif(GMP_FOUND)
        target_link_libraries(${TARGET} PRIVATE ${GMP_LIB})
    endif()

    if (MSVC)
        target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:/MT>)
        target_link_options(${TARGET} PRIVATE /SAFESEH:NO)
    elseif(MINGW)
        target_link_options(${TARGET} PRIVATE -static -lwinpthread)
    endif()
endforeach()
