cmake_minimum_required(VERSION 3.14)
project(l2encdec_cli VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME} cli.cpp)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

set(L2ENCDEC_OUTPUT_NAME
    "l2encdec"
    CACHE STRING "Output name"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME ${L2ENCDEC_OUTPUT_NAME}
)

if(MINGW AND NOT BUILD_SHARED_LIBS)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -lwinpthread")
endif()

if(NOT L2ENCDEC_DIR)
    set(L2ENCDEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
    add_subdirectory(${L2ENCDEC_DIR} ${CMAKE_CURRENT_BINARY_DIR}/l2encdec)
endif()
if(MSVC)
    add_subdirectory(extern/getopt)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE l2encdec $<$<BOOL:${MSVC}>:getopt>)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION . COMPONENT cli
    BUNDLE DESTINATION . COMPONENT cli
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    ${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE
    DESTINATION .
    COMPONENT cli
)

set(CPACK_GENERATOR ZIP)
set(CPACK_PACKAGE_NAME ${L2ENCDEC_OUTPUT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

set(CPACK_COMPONENTS_ALL cli)

include(CPack)
