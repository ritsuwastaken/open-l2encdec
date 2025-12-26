cmake_minimum_required(VERSION 3.28)
project(l2encdec VERSION 2.0.0 LANGUAGES C CXX)

include(FetchContent)
include(GenerateExportHeader)

set(L2ENCDEC_THREADS_DEFAULT ON)
if(EMSCRIPTEN)
    set(L2ENCDEC_THREADS_DEFAULT OFF)
endif()
option(L2ENCDEC_ENABLE_THREADS "Process RSA blocks in parallel" ${L2ENCDEC_THREADS_DEFAULT})
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(L2ENCDEC_BUILD_CLI "Build the command-line tool" ${PROJECT_IS_TOP_LEVEL})
option(L2ENCDEC_BUILD_TESTS "Build tests" ${PROJECT_IS_TOP_LEVEL})
option(L2ENCDEC_BUILD_TS_BINDINGS "Build typescript bindings" OFF)

fetchcontent_declare(
    libtomcrypt
    GIT_REPOSITORY https://github.com/libtom/libtomcrypt.git
    GIT_TAG 6c6d5104de66f3ca0dfd7b68540ef86869982b07
    EXCLUDE_FROM_ALL
)
fetchcontent_declare(
    libtommath
    GIT_REPOSITORY https://github.com/libtom/libtommath.git
    GIT_TAG ae40a87a920099a7d9d00979570e0c8d917a1fd7
    EXCLUDE_FROM_ALL
)
fetchcontent_declare(
    zlibng
    GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng.git
    GIT_TAG 2.3.3
    GIT_SHALLOW TRUE
    EXCLUDE_FROM_ALL
)
block()
    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) # libtommath
    set(BUILD_SHARED_LIBS OFF)
    set(BUILD_TESTING OFF) # libtomcrypt, libtommath, zlibng
    set(WITH_LTM OFF) # libtomcrypt
    set(CMAKE_C_VISIBILITY_PRESET hidden)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
    fetchcontent_makeavailable(libtomcrypt libtommath zlibng)
endblock()

set_target_properties(libtomcrypt libtommath zlib-ng PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(${PROJECT_NAME}
    src/l2encdec.cpp
    src/blowfish.cpp
    src/rsa.cpp
    src/utils.cpp
    src/xor_utils.cpp
    src/zlib_utils.cpp
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES
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
target_compile_definitions(${PROJECT_NAME} PRIVATE
    L2ENCDEC_ENABLE_THREADS=$<BOOL:${L2ENCDEC_ENABLE_THREADS}>
)
if(L2ENCDEC_ENABLE_THREADS)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif()

if(L2ENCDEC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(L2ENCDEC_BUILD_TS_BINDINGS)
    add_subdirectory(modules/typescript)
endif()

if(L2ENCDEC_BUILD_CLI)
    add_subdirectory(cli)
endif()
