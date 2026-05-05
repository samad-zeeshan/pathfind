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
    src/search_core.cpp
    src/bfs.cpp
    src/algorithms.cpp
)
target_link_libraries(pathfind PRIVATE raylib)

if(WIN32)
    target_link_libraries(pathfind PRIVATE winmm)
endif()

# Web build: emcmake cmake -S . -B build-web -DPLATFORM=Web -DCMAKE_BUILD_TYPE=Release
# The PLATFORM var is consumed by raylib's CMake, EMSCRIPTEN is set by the emsdk toolchain.
if(EMSCRIPTEN)
    target_compile_definitions(pathfind PRIVATE PLATFORM_WEB)
    set_target_properties(pathfind PROPERTIES SUFFIX ".html")
    target_link_options(pathfind PRIVATE
        -sUSE_GLFW=3
        -sALLOW_MEMORY_GROWTH=1
        --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html)
    # Relink when the shell changes, CMake does not track --shell-file itself.
    set_target_properties(pathfind PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html)
endif()