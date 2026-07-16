add_library(engine STATIC
    core/engine.cpp
    renderer/renderer.cpp
)

target_include_directories(engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Ensure math header is available
target_include_directories(engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/math
)

if(EMSCRIPTEN)
    target_compile_options(engine PRIVATE -s USE_SDL=2 -s USE_WEBGL2=1)
endif()
