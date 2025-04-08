cmake_minimum_required(VERSION 3.28)
project(l2encdec_cli VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(L2ENCDEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

add_executable(${PROJECT_NAME} src/cli.cpp)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
if(MINGW AND NOT BUILD_SHARED_LIBS)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -lwinpthread")
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "l2encdec$<$<PLATFORM_ID:Darwin>:_osx>$<$<PLATFORM_ID:Windows>:_win>$<$<PLATFORM_ID:Linux>:_linux>"
)
add_subdirectory(${L2ENCDEC_DIR} ${CMAKE_CURRENT_BINARY_DIR}/l2encdec)
if(MSVC)
    add_subdirectory(${L2ENCDEC_DIR}/extern/getopt ${CMAKE_CURRENT_BINARY_DIR}/getopt)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE l2encdec $<$<BOOL:${MSVC}>:getopt>)
