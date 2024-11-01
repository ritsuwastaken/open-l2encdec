cmake_minimum_required(VERSION 3.11)
project(l2encdec VERSION 1.0.4 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(SOURCES
    src/l2encdec.cpp
    src/rsa.cpp
    src/xor.cpp
    src/bf.cpp
    src/zlib_utils.cpp
    blowfish/blowfish.cpp
    miniz/miniz.c
)

set(INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    blowfish
    miniz
)

if(MSVC)
    list(APPEND SOURCES getopt/getopt.c)
    list(APPEND INCLUDES getopt)
endif()

if(NOT UNIX)
    set(GMP_USE_STATIC_LIBS ON)
endif()
find_package(GMP QUIET)
if(GMP_FOUND)
    list(APPEND INCLUDES ${GMP_INCLUDE_DIR})
    add_compile_definitions(GMP_FOUND)
else()
    list(APPEND SOURCES mini-gmp/mini-gmp.c)
    list(APPEND INCLUDES mini-gmp)
endif()

add_library(${PROJECT_NAME} SHARED ${SOURCES})
add_executable(${PROJECT_NAME}_exe src/main.cpp ${SOURCES})

if(APPLE)
    set_target_properties(${PROJECT_NAME}_exe PROPERTIES OUTPUT_NAME "${PROJECT_NAME}_osx")
else()
    set_target_properties(${PROJECT_NAME}_exe PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
endif()

foreach(TARGET ${PROJECT_NAME} ${PROJECT_NAME}_exe)
    target_include_directories(${TARGET} PRIVATE ${INCLUDES})
    target_compile_definitions(${TARGET} PRIVATE L2ENCDEC_EXPORTS)
    
    if(GMP_FOUND)
        target_link_libraries(${TARGET} PRIVATE ${GMP_LIB})
    endif()
    
    if(MSVC)
        target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:/MT>)
    elseif(NOT APPLE)
        target_link_options(${TARGET} PRIVATE -static-libgcc -static-libstdc++)
        if(WIN32)
            target_link_options(${TARGET} PRIVATE -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,-Bdynamic,--no-whole-archive)
        endif()
    endif()
endforeach()
