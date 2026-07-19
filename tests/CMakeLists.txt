cmake_minimum_required(VERSION 3.14)
project(l2encdec_tests LANGUAGES CXX)

include(FetchContent)

fetchcontent_declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    GIT_SHALLOW TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(INSTALL_GMOCK OFF CACHE BOOL "" FORCE)
fetchcontent_makeavailable(googletest)

add_executable(${PROJECT_NAME}
    test_xor.cpp
    test_blowfish.cpp
    test_zlib.cpp
    test_rsa.cpp
    test_utils.cpp
    test_l2encdec.cpp
    test_l2encdec_init_params.cpp
    test_l2encdec_verify_checksum.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    gtest_main
    l2encdec
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

include(GoogleTest)
gtest_add_tests(TARGET ${PROJECT_NAME})
