cmake_minimum_required(VERSION 3.28)
project(l2encdec VERSION 1.0.15 LANGUAGES CXX C)

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
    ${OPENSSL_INCLUDE_DIR}
)

set(EXE_SOURCES src/main.cpp)

set(EXE_INCLUDES "")

if(MSVC)
    list(APPEND EXE_SOURCES third_party/getopt/getopt.c)
    list(APPEND EXE_INCLUDES third_party/getopt)
endif()

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDES})
target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::Crypto)

if(BUILD_SHARED_LIBS)
    if(NOT WIN32)
        target_compile_options(${PROJECT_NAME} PRIVATE -fvisibility=hidden)
    endif()
    target_compile_definitions(${PROJECT_NAME} 
        PRIVATE L2ENCDEC_EXPORTS
        PUBLIC L2ENCDEC_SHARED)
endif()

add_executable(${PROJECT_NAME}_exe ${EXE_SOURCES})
target_include_directories(${PROJECT_NAME}_exe PRIVATE ${EXE_INCLUDES} ${INCLUDES})
target_link_libraries(${PROJECT_NAME}_exe PRIVATE ${PROJECT_NAME} OpenSSL::Crypto)
target_compile_definitions(${PROJECT_NAME}_exe PRIVATE L2ENCDEC_STATIC L2ENCDEC_NO_EXPORT L2ENCDEC_NO_IMPORT)
set_target_properties(${PROJECT_NAME}_exe PROPERTIES OUTPUT_NAME "${PROJECT_NAME}$<$<PLATFORM_ID:Darwin>:_osx>")

foreach(TARGET ${PROJECT_NAME} ${PROJECT_NAME}_exe)
    if (MSVC)
        target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:/MT>)
        target_link_options(${TARGET} PRIVATE /SAFESEH:NO)
    elseif(MINGW)
        target_link_options(${TARGET} PRIVATE -static -lwinpthread)
    endif()
endforeach()
