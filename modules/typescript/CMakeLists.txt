cmake_minimum_required(VERSION 3.14)
project(l2encdec-typescript)

if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "This target must be built with Emscripten")
endif()

if(NOT DEFINED L2ENCDEC_DIR)
    set(L2ENCDEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
    add_subdirectory(${L2ENCDEC_DIR} ${CMAKE_CURRENT_BINARY_DIR}/l2encdec EXCLUDE_FROM_ALL)
endif()

set(TSD_FILE ${TS_OUTPUT_DIR}/interface.d.ts)
set(CMAKE_EXECUTABLE_SUFFIX ".js")
set(TS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/dist)
file(MAKE_DIRECTORY ${TS_OUTPUT_DIR})

add_executable(${PROJECT_NAME} bindings.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${L2ENCDEC_DIR}/include
    ${L2ENCDEC_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--whole-archive l2encdec -Wl,--no-whole-archive)

target_compile_options(${PROJECT_NAME} PRIVATE -O3)

target_link_options(${PROJECT_NAME} PRIVATE
    "-lembind"
    "-sMODULARIZE"
    "-sEXPORT_ES6"
    "-sASSERTIONS"
    "-sALLOW_MEMORY_GROWTH"
    "--no-entry"
    "--emit-tsd=${TSD_FILE}"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${TS_OUTPUT_DIR}
    OUTPUT_NAME "l2encdec"
    CXX_STANDARD 20
)
