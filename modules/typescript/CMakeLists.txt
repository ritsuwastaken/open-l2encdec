if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "This target must be built with Emscripten")
endif()

set(TS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/dist)

foreach(environment IN ITEMS browser node)
    if(environment STREQUAL "browser")
        set(runtime web)
    else()
        set(runtime node)
    endif()
    set(target l2encdec_${environment})
    set(declarations ${TS_OUTPUT_DIR}/l2encdec.${environment}.d.ts)
    add_executable(${target} bindings.cpp)
    target_link_libraries(${target} PRIVATE l2encdec embind)
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_link_options(${target} PRIVATE
        --no-entry
        -sMODULARIZE=1
        -sEXPORT_ES6=1
        -sSINGLE_FILE=1
        -sALLOW_MEMORY_GROWTH=1
        -sENVIRONMENT=${runtime}
        --emit-tsd=${declarations}
    )
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${TS_OUTPUT_DIR}
        OUTPUT_NAME l2encdec.${environment}
        SUFFIX .js
    )
    install(TARGETS ${target} RUNTIME DESTINATION dist COMPONENT typescript)
    install(FILES ${declarations} DESTINATION dist COMPONENT typescript)
endforeach()
