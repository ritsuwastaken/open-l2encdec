# Force static CRT on MSVC builds
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "MSVC runtime" FORCE)
