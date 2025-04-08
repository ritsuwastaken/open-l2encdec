cmake_minimum_required(VERSION 3.28)
project(txt211json VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME} src/main.cpp src/utf.cpp src/normalize.cpp src/json.cpp)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

if(NOT L2ENCDEC_DIR)
    set(L2ENCDEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
    add_subdirectory(${L2ENCDEC_DIR} ${CMAKE_CURRENT_BINARY_DIR}/l2encdec)
endif()

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# not the intended way to install utfcpp, but it works for this version
FetchContent_Declare(
    utfcpp
    GIT_REPOSITORY "https://github.com/nemtrif/utfcpp.git"
    GIT_TAG v4.0.6
)

set(UTF8_TESTS OFF CACHE BOOL "" FORCE)
set(UTF8_SAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(utfcpp)

target_link_libraries(${PROJECT_NAME} PRIVATE l2encdec nlohmann_json::nlohmann_json)
target_include_directories(${PROJECT_NAME} PRIVATE ${utfcpp_SOURCE_DIR}/source)
