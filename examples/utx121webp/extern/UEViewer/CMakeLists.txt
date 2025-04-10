project(UEViewer VERSION 1.0.0 LANGUAGES CXX)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(UEVIEWER_SOURCES
    src/Core.cpp
    src/CoreWin32.cpp
    src/GameFileSystem.cpp
    src/Memory.cpp
    src/TypeInfo.cpp
    src/UnCore.cpp
    src/UnCoreSerialize.cpp
    src/UnObject.cpp
    src/UnPackage.cpp
    src/UnPackage2.cpp
    src/UnTexture2.cpp
)

add_library(${PROJECT_NAME} STATIC ${UEVIEWER_SOURCES} src/ueviewer.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /EHs- /GR-)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _HAS_EXCEPTIONS=0
        NO_EXCEPTION_HANDLING
    )
endif()
