cmake_minimum_required(VERSION 3.28)
project(l2encdec_example VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Switch off to link l2encdec statically
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# Using FetchContent:
include(FetchContent)
FetchContent_Declare(
    l2encdec
    GIT_REPOSITORY https://github.com/ritsuwastaken/open-l2encdec.git
    GIT_TAG main
)
FetchContent_MakeAvailable(l2encdec)
# Alternatively, when used as a subproject:
# set(L2ENCDEC_DIR ${CMAKE_SOURCE_DIR}/..)
# add_subdirectory(${L2ENCDEC_DIR} ${CMAKE_BINARY_DIR}/l2encdec)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE l2encdec)
