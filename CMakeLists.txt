cmake_minimum_required(VERSION 3.20)
project(pathfinder CXX)

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

# Everything the searches need and nothing the renderer does, shared by the
# app, the tests, and the benchmark.
set(PATHFINDER_CORE_SOURCES
    src/grid.cpp
    src/search_core.cpp
    src/bfs.cpp
    src/bidirectional.cpp
    src/jps.cpp
    src/algorithms.cpp
)

add_executable(pathfinder
    src/main.cpp
    src/config.cpp
    src/renderer.cpp
    src/ui.cpp
    src/controller.cpp
    ${PATHFINDER_CORE_SOURCES}
)
target_link_libraries(pathfinder PRIVATE raylib)

if(WIN32)
    target_link_libraries(pathfinder PRIVATE winmm)
endif()

# Headless benchmark over the committed fixtures in maps/. No raylib.
if(NOT EMSCRIPTEN)
    add_executable(pathfinder_bench src/bench_main.cpp ${PATHFINDER_CORE_SOURCES})
endif()

# Pure-logic test suite, no raylib. Meaningless in a wasm build.
option(PATHFINDER_BUILD_TESTS "Build the unit tests" ON)
if(PATHFINDER_BUILD_TESTS AND NOT EMSCRIPTEN)
    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG        v2.4.12
    )
    FetchContent_MakeAvailable(doctest)

    enable_testing()
    add_executable(pathfinder_tests tests/test_pathfinding.cpp ${PATHFINDER_CORE_SOURCES})
    target_include_directories(pathfinder_tests PRIVATE src)
    target_link_libraries(pathfinder_tests PRIVATE doctest::doctest)
    add_test(NAME pathfinder_tests COMMAND pathfinder_tests)
endif()

# Web build: emcmake cmake -S . -B build-web -DPLATFORM=Web -DCMAKE_BUILD_TYPE=Release
# The PLATFORM var is consumed by raylib's CMake, EMSCRIPTEN is set by the emsdk toolchain.
if(EMSCRIPTEN)
    target_compile_definitions(pathfinder PRIVATE PLATFORM_WEB)
    set_target_properties(pathfinder PROPERTIES SUFFIX ".html")
    target_link_options(pathfinder PRIVATE
        -sUSE_GLFW=3
        -sALLOW_MEMORY_GROWTH=1
        --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html)
    # Relink when the shell changes, CMake does not track --shell-file itself.
    set_target_properties(pathfinder PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html)
endif()
