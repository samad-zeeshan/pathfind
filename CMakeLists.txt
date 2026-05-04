cmake_minimum_required(VERSION 3.20)
project(pathfind CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(CMAKE_POLICY_VERSION_MINIMUM 3.10 CACHE STRING "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(raylib)

add_executable(pathfind
    src/main.cpp
    src/grid.cpp
    src/renderer.cpp
)
target_link_libraries(pathfind PRIVATE raylib)

if(WIN32)
    target_link_libraries(pathfind PRIVATE winmm)
endif()