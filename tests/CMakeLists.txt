fetchcontent_declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.18.0
    GIT_SHALLOW TRUE
    EXCLUDE_FROM_ALL
)
fetchcontent_makeavailable(googletest)

add_executable(l2encdec_tests
    test_xor.cpp
    test_blowfish.cpp
    test_zlib.cpp
    test_rsa.cpp
    test_utils.cpp
    test_l2encdec.cpp
    test_l2encdec_init_params.cpp
    test_l2encdec_verify_checksum.cpp
)

target_compile_features(l2encdec_tests PRIVATE cxx_std_20)
set_target_properties(l2encdec_tests PROPERTIES CXX_EXTENSIONS OFF)
target_include_directories(l2encdec_tests PRIVATE ../src)
target_link_libraries(l2encdec_tests PRIVATE GTest::gtest_main l2encdec)

include(GoogleTest)
gtest_add_tests(TARGET l2encdec_tests)
