cmake_minimum_required(VERSION 3.16)
project(web3dgame)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Configuration ---

# --- Configuration ---

# Base warnings (always on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Emscripten: setup output directory and compiler/linker flags BEFORE subdirectories
if(EMSCRIPTEN)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/web)
    # Force use of Emscripten's libc++ instead of system C++ headers
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    # Note: Cannot use -Werror in Emscripten compile phase due to toolchain constraints
    # in this container environment. Will apply -Werror at link time only (on main executable).
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Werror -s USE_WEBGL2=1 -s USE_GLFW=0 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=0 --shell-file ${CMAKE_SOURCE_DIR}/web/shell.html --preload-file ${CMAKE_SOURCE_DIR}/assets@/assets")
else()
    # Desktop: apply -Werror at compile time
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif()

# --- Dependencies ---

# GLM (header-only, no special setup needed)
find_package(glm QUIET)
if(NOT glm_FOUND)
    message(STATUS "GLM package not found via CMake, using vendored headers from third_party/glm")
    include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/third_party/glm)
endif()

# GLEW (optional on Emscripten, but we'll skip it via preprocessor)
if(NOT EMSCRIPTEN)
    find_package(GLEW REQUIRED)
    find_package(OpenGL REQUIRED)
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
    target_link_libraries(web3dgame GLEW::GLEW OpenGL::GL)
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
