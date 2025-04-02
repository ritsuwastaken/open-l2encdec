cmake_minimum_required(VERSION 3.28)
project(l2encdec_example VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME} src/main.cpp)
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

FetchContent_Declare(
    l2encdec
    GIT_REPOSITORY https://github.com/ritsuwastaken/open-l2encdec.git
    GIT_TAG 1.1.9
)
FetchContent_MakeAvailable(l2encdec)

target_link_libraries(${PROJECT_NAME} PRIVATE l2encdec)
