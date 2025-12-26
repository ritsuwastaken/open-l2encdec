cmake_minimum_required(VERSION 3.14)
project(l2encdec-typescript)

if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "This target must be built with Emscripten")
endif()

if(NOT DEFINED L2ENCDEC_DIR)
    set(L2ENCDEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
    add_subdirectory(${L2ENCDEC_DIR} ${CMAKE_CURRENT_BINARY_DIR}/l2encdec)
endif()

set(CMAKE_EXECUTABLE_SUFFIX ".js")
set(TS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/dist)
file(MAKE_DIRECTORY ${TS_OUTPUT_DIR})

set(TSD_FILE ${TS_OUTPUT_DIR}/l2encdec.js.d.ts)

set(COMMON_LINK_FLAGS
    "-lembind"
    "-sMODULARIZE=1"
    "-sEXPORT_ES6=1"
    "-sSINGLE_FILE=1"
    "-sALLOW_MEMORY_GROWTH=1"
    "--no-entry"
)

# browser
add_executable(l2encdec_browser bindings.cpp)

target_link_libraries(l2encdec_browser PRIVATE
    -Wl,--whole-archive l2encdec -Wl,--no-whole-archive
)

target_compile_options(l2encdec_browser PRIVATE -O3)

target_link_options(l2encdec_browser PRIVATE
    ${COMMON_LINK_FLAGS}
    "-sENVIRONMENT=web"
    "--emit-tsd=${TSD_FILE}"
)

set_target_properties(l2encdec_browser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${TS_OUTPUT_DIR}
    OUTPUT_NAME "l2encdec.browser"
    CXX_STANDARD 20
)

# node
add_executable(l2encdec_node bindings.cpp)

target_link_libraries(l2encdec_node PRIVATE
    -Wl,--whole-archive l2encdec -Wl,--no-whole-archive
)

target_compile_options(l2encdec_node PRIVATE -O3)

target_link_options(l2encdec_node PRIVATE
    ${COMMON_LINK_FLAGS}
    "-sENVIRONMENT=node"
)

set_target_properties(l2encdec_node PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${TS_OUTPUT_DIR}
    OUTPUT_NAME "l2encdec.node"
    CXX_STANDARD 20
)
