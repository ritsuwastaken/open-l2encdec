get_target_property(CPACK_PACKAGE_NAME l2encdec NAME)
get_target_property(CPACK_PACKAGE_VERSION l2encdec VERSION)

add_executable(l2encdec_cli cli.cpp)
target_compile_features(l2encdec_cli PRIVATE cxx_std_20)
target_link_libraries(l2encdec_cli PRIVATE l2encdec)
set_target_properties(l2encdec_cli PROPERTIES OUTPUT_NAME ${CPACK_PACKAGE_NAME})

install(TARGETS l2encdec_cli DESTINATION .)
install(FILES README.md ../LICENSE DESTINATION .)

include(CPack)
