cmake_minimum_required(VERSION 3.28)
project(l2encdec_cli VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(L2ENCDEC_DIR ${CMAKE_SOURCE_DIR}/../..)

add_executable(${PROJECT_NAME} src/cli.cpp)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
set_target_properties(${PROJECT_NAME} PROPERTIES 
    OUTPUT_NAME "l2encdec$<$<PLATFORM_ID:Darwin>:_osx>$<$<PLATFORM_ID:Windows>:_win>$<$<PLATFORM_ID:Linux>:_linux>"
)
add_subdirectory(${L2ENCDEC_DIR} ${CMAKE_BINARY_DIR}/l2encdec)
if(MSVC)
    add_subdirectory(${L2ENCDEC_DIR}/3rd_party/getopt ${CMAKE_BINARY_DIR}/getopt)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE l2encdec $<$<BOOL:${MSVC}>:getopt>)
