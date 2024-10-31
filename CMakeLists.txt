cmake_minimum_required(VERSION 3.11...3.30)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(l2encdec VERSION 1.0.4 LANGUAGES CXX C)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

set(GETOPT_MS_SOURCES getopt/getopt.c getopt/getopt.h)
set(BLOWFISH_SOURCES blowfish/blowfish.cpp blowfish/blowfish.h)
set(MINIZ_SOURCES miniz/miniz.c miniz/miniz.h)
set(MINI_GMP_SOURCES mini-gmp/mini-gmp.c mini-gmp/mini-gmp.h)
set(COMMON_SOURCES src/l2encdec.cpp src/rsa.cpp src/rsa.h src/xor.cpp src/xor.h src/bf.cpp src/bf.h src/zlib_utils.cpp src/zlib_utils.h ${MINIZ_SOURCES} ${BLOWFISH_SOURCES})
set(COMMON_INCLUDE_DIRS blowfish miniz)

# Getopt support for MSVC
if (MSVC)
  list(APPEND COMMON_INCLUDE_DIRS getopt)
endif()

# GMP
if(NOT UNIX)
  set(GMP_USE_STATIC_LIBS ON)
endif()
find_package(GMP QUIET)
if (GMP_FOUND)
    message(STATUS "GMP library: ${GMP_LIB}")
else()
    message(STATUS "GMP not found, using mini-gmp")
endif()

# Threads
find_package(Threads REQUIRED)

# Header
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/l2encdec.h
    ${CMAKE_BINARY_DIR}/include/l2encdec.h
    COPYONLY
)

# Library
add_library(${PROJECT_NAME} SHARED ${COMMON_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# Executable
add_executable(${PROJECT_NAME}_exe src/main.cpp ${COMMON_SOURCES})
set(EXECUTABLE_NAME ${PROJECT_NAME})
if (APPLE)
  set(EXECUTABLE_NAME ${PROJECT_NAME}_osx)
endif()
set_target_properties(${PROJECT_NAME}_exe PROPERTIES OUTPUT_NAME ${EXECUTABLE_NAME})

# Link libs statically when possible, link gmp if found
foreach(TARGET ${PROJECT_NAME} ${PROJECT_NAME}_exe)
  if(NOT APPLE AND NOT MSVC)
    if(WIN32)
      target_link_options(${TARGET} PRIVATE -static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,-Bdynamic,--no-whole-archive)
    else()
      target_link_options(${TARGET} PRIVATE -static-libgcc -static-libstdc++)
    endif()
  elseif(MSVC)
    target_compile_options(${TARGET} PRIVATE
      $<$<CONFIG:>:/MT>
      $<$<CONFIG:Debug>:/MTd>
      $<$<CONFIG:Release>:/MT>
    )
    # Link ms getopt source if building executable
    if(TARGET ${PROJECT_NAME}_exe)
      target_sources(${TARGET} PRIVATE ${GETOPT_MS_SOURCES})
    endif()
  endif()
  if(GMP_FOUND)
    target_include_directories(${TARGET} PRIVATE 
        ${GMP_INCLUDE_DIR}
        ${COMMON_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(${TARGET} PRIVATE ${CMAKE_THREAD_LIBS_INIT} ${GMP_LIB})
    target_compile_definitions(${TARGET} PRIVATE L2ENCDEC_EXPORTS GMP_FOUND)
  else()
    target_include_directories(${TARGET} PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src 
        ${COMMON_INCLUDE_DIRS} 
        mini-gmp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(${TARGET} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
    target_sources(${TARGET} PRIVATE ${MINI_GMP_SOURCES})
    target_compile_definitions(${TARGET} PRIVATE L2ENCDEC_EXPORTS)
  endif()
endforeach()
