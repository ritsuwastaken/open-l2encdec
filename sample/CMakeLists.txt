cmake_minimum_required(VERSION 3.28)
project(l2encdec_sample VERSION 1.0.0 LANGUAGES CXX)

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
# find_path(L2ENCDEC_INCLUDE_DIR
#     NAMES l2encdec.h
#     PATHS ${L2ENCDEC_DIR}/include
#     NO_DEFAULT_PATH
# )

add_executable(l2encdec_sample src/sample.cpp)
target_link_libraries(l2encdec_sample PRIVATE l2encdec)
# Using FetchContent:
target_include_directories(l2encdec_sample PRIVATE ${l2encdec_SOURCE_DIR}/include)
# As a subproject:
# target_include_directories(l2encdec_sample PRIVATE ${L2ENCDEC_INCLUDE_DIR})
