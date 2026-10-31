cmake_minimum_required(VERSION 3.28)
project(l2encdec_cli VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME} cli.cpp)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

set(L2ENCDEC_OUTPUT_NAME "l2encdec" CACHE STRING "Output name")
set(CPACK_PACKAGE_NAME ${L2ENCDEC_OUTPUT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME ${L2ENCDEC_OUTPUT_NAME}
)

if(NOT TARGET l2encdec)
    set(L2ENCDEC_BUILD_CLI OFF)
    add_subdirectory(.. l2encdec-build)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE l2encdec)

install(TARGETS ${PROJECT_NAME} DESTINATION .)
install(FILES README.md ../LICENSE DESTINATION .)

get_target_property(CPACK_PACKAGE_VERSION l2encdec VERSION)

include(CPack)
