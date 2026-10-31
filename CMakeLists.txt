cmake_minimum_required(VERSION 3.28)
project(l2encdec VERSION 1.5.0 LANGUAGES C CXX)

include(FetchContent)
include(GenerateExportHeader)
include(CTest)

fetchcontent_declare(
    libtomcrypt
    GIT_REPOSITORY https://github.com/libtom/libtomcrypt.git
    GIT_TAG develop
    GIT_SHALLOW TRUE
    EXCLUDE_FROM_ALL
)
fetchcontent_declare(
    libtommath
    GIT_REPOSITORY https://github.com/libtom/libtommath.git
    GIT_TAG develop
    GIT_SHALLOW TRUE
    EXCLUDE_FROM_ALL
)
fetchcontent_declare(
    zlibng
    GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng.git
    GIT_TAG 2.3.3
    EXCLUDE_FROM_ALL
    GIT_SHALLOW TRUE
)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE) # dependencies
set(WITH_LTM OFF CACHE BOOL "" FORCE) # libtomcrypt
fetchcontent_makeavailable(libtomcrypt libtommath zlibng)

set_target_properties(libtomcrypt libtommath zlib-ng PROPERTIES POSITION_INDEPENDENT_CODE ON)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(L2ENCDEC_BUILD_TESTS "Build unit tests" OFF)
option(L2ENCDEC_BUILD_CLI "Build examples" OFF)

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

target_link_libraries(${PROJECT_NAME} PRIVATE libtomcrypt libtommath zlib-ng::zlib)

if(L2ENCDEC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(L2ENCDEC_BUILD_CLI)
    add_subdirectory(cli)
endif()
