cmake_minimum_required(VERSION 3.16)
project(web3dgame)

# --- Configuration ---

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

# Emscripten-specific flags
if(EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2 -s USE_WEBGL2=1 -s USE_GLFW=0")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=0")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --shell-file ${CMAKE_SOURCE_DIR}/web/shell.html")
    
    # Output to web/ directory
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/web)
endif()

# --- Dependencies ---

# GLM (header-only, no special setup needed)
if(NOT TARGET glm)
    # Assume glm is installed system-wide or bundled
    find_package(glm QUIET)
    if(NOT glm_FOUND)
        message(STATUS "GLM not found, assuming header-only installation")
        include_directories(/usr/include)
    endif()
endif()

# GLEW (optional on Emscripten, but we'll skip it via preprocessor)
if(NOT EMSCRIPTEN)
    find_package(GLEW REQUIRED)
endif()

# --- Subdirectories ---

add_subdirectory(src/engine)
add_subdirectory(src/game)
add_subdirectory(src/net)

# --- Main executable ---

add_executable(web3dgame
    src/main.cpp
)

target_link_libraries(web3dgame
    engine
    game
    net
)

if(NOT EMSCRIPTEN)
    target_link_libraries(web3dgame GLEW::GLEW)
endif()

# --- Build output configuration ---

if(EMSCRIPTEN)
    # Set output name (Emscripten will add .html, .wasm, .js)
    set_target_properties(web3dgame PROPERTIES SUFFIX ".html")
endif()

message(STATUS "Build configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  CXX Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "  Emscripten: ${EMSCRIPTEN}")
