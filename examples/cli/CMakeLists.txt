cmake_minimum_required(VERSION 3.14)
project(l2encdec_cli VERSION 1.0.0 LANGUAGES CXX)

include(CPack)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME} src/cli.cpp)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
if(NOT L2ENCDEC_DIR)
    set(L2ENCDEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
    add_subdirectory(${L2ENCDEC_DIR} ${CMAKE_CURRENT_BINARY_DIR}/l2encdec)
endif()
if(MINGW AND NOT BUILD_SHARED_LIBS)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -lwinpthread")
endif()
if(MSVC)
    add_subdirectory(extern/getopt)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE l2encdec $<$<BOOL:${MSVC}>:getopt>)

install(TARGETS ${PROJECT_NAME} COMPONENT cli)
