cmake_minimum_required(VERSION 3.28)
project(l2encdec VERSION 1.1.6 LANGUAGES CXX C)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(L2ENCDEC_BUILD_LIB "Build library" ON)
option(L2ENCDEC_BUILD_EXE "Build executable" OFF)

if(NOT L2ENCDEC_BUILD_LIB AND NOT L2ENCDEC_BUILD_EXE)
    message(FATAL_ERROR "At least one of L2ENCDEC_BUILD_LIB or L2ENCDEC_BUILD_EXE must be ON")
endif()

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
set(USER_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force dependencies to build statically" FORCE)
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
    GIT_TAG        ae07a3615925f252f54fecb84aee2e3cfd4f0676
)
FetchContent_MakeAvailable(miniz)

add_subdirectory(3rd_party/blowfish)
if(MSVC AND L2ENCDEC_BUILD_EXE MATCHES ON)
    add_subdirectory(3rd_party/getopt)
endif()

set(BUILD_SHARED_LIBS ${USER_BUILD_SHARED_LIBS} CACHE BOOL "Restore user preference" FORCE)

set(SOURCES
    src/l2encdec.cpp
    src/rsa.cpp
    src/xor.cpp
    src/bf.cpp
    src/zlib_utils.cpp
)

if(L2ENCDEC_BUILD_LIB)
    add_library(${PROJECT_NAME} ${SOURCES})
    target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(${PROJECT_NAME} PRIVATE mbedcrypto miniz blowfish)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set_target_properties(${PROJECT_NAME} PROPERTIES
            CXX_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN ON
        )
    endif()
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(${PROJECT_NAME} PRIVATE L2ENCDEC_EXPORTS)
    endif()
endif()

if(L2ENCDEC_BUILD_EXE)
    add_executable(${PROJECT_NAME}_exe src/cli.cpp ${SOURCES})
    target_include_directories(${PROJECT_NAME}_exe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(${PROJECT_NAME}_exe PRIVATE mbedcrypto miniz blowfish $<$<BOOL:${MSVC}>:getopt>)
    target_compile_definitions(${PROJECT_NAME}_exe PRIVATE L2ENCDEC_STATIC)
    set_target_properties(${PROJECT_NAME}_exe PROPERTIES 
        OUTPUT_NAME "${PROJECT_NAME}$<$<PLATFORM_ID:Darwin>:_osx>$<$<PLATFORM_ID:Windows>:_win>"
    )
endif()
