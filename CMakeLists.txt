cmake_minimum_required(VERSION 3.28)
project(l2encdec VERSION 1.4.0 LANGUAGES C CXX)

include(FetchContent)
include(GenerateExportHeader)
include(CTest)

fetchcontent_declare(
    libtomcrypt
    GIT_REPOSITORY https://github.com/libtom/libtomcrypt.git
    GIT_TAG develop
    GIT_SHALLOW TRUE
)
fetchcontent_declare(
    libtommath
    GIT_REPOSITORY https://github.com/libtom/libtommath.git
    GIT_TAG develop
    GIT_SHALLOW TRUE
)
fetchcontent_declare(
    miniz
    GIT_REPOSITORY https://github.com/richgel999/miniz.git
    GIT_TAG 3.1.2
    GIT_SHALLOW TRUE
)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE) # libtomcrypt, libtommath
set(WITH_LTM OFF CACHE BOOL "" FORCE) # libtomcrypt
fetchcontent_makeavailable(libtomcrypt libtommath miniz)

set_property(DIRECTORY ${libtomcrypt_SOURCE_DIR} PROPERTY EXCLUDE_FROM_ALL TRUE)
set_property(DIRECTORY ${libtommath_SOURCE_DIR} PROPERTY EXCLUDE_FROM_ALL TRUE)

set_target_properties(libtomcrypt libtommath miniz PROPERTIES POSITION_INDEPENDENT_CODE ON)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(L2ENCDEC_BUILD_TESTS "Build unit tests" OFF)
option(L2ENCDEC_BUILD_CLI "Build examples" OFF)

set(L2ENCDEC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

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
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

generate_export_header(${PROJECT_NAME}
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/include/l2encdec-export.h
    EXPORT_MACRO_NAME L2ENCDEC_API
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE libtomcrypt libtommath miniz)

if(L2ENCDEC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(L2ENCDEC_BUILD_CLI)
    add_subdirectory(cli)
endif()
