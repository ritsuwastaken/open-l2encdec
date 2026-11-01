if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "This target must be built with Emscripten")
endif()

add_executable(l2encdec_wasm bindings.cpp)
target_link_libraries(l2encdec_wasm PRIVATE l2encdec embind)
target_compile_features(l2encdec_wasm PRIVATE cxx_std_20)
target_link_options(l2encdec_wasm PRIVATE
    --no-entry
    -sMODULARIZE=1
    -sEXPORT_ES6=1
    -sSINGLE_FILE=1
    -sALLOW_MEMORY_GROWTH=1
    -sENVIRONMENT=web,worker,node
    --emit-tsd=${CMAKE_CURRENT_BINARY_DIR}/dist/l2encdec.d.ts
)
set_target_properties(l2encdec_wasm PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist
    OUTPUT_NAME l2encdec
    SUFFIX .js
)

install(TARGETS l2encdec_wasm RUNTIME DESTINATION dist COMPONENT typescript)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dist/l2encdec.d.ts DESTINATION dist COMPONENT typescript)
