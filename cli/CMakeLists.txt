cmake_minimum_required(VERSION 3.28)
project(l2encdec_cli VERSION 1.0.0 LANGUAGES CXX)

set(L2ENCDEC_OUTPUT_NAME "l2encdec" CACHE STRING "Output name")
set(CPACK_PACKAGE_NAME ${L2ENCDEC_OUTPUT_NAME})
get_target_property(CPACK_PACKAGE_VERSION l2encdec VERSION)

add_executable(${PROJECT_NAME} cli.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE l2encdec)

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME ${L2ENCDEC_OUTPUT_NAME}
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

install(TARGETS ${PROJECT_NAME} DESTINATION .)
install(FILES README.md ../LICENSE DESTINATION .)

include(CPack)
