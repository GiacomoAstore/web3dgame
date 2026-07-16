add_library(engine STATIC
    core/engine.cpp
    renderer/renderer.cpp
)

target_include_directories(engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Ensure math header and third-party headers are available
target_include_directories(engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/math
    ${CMAKE_SOURCE_DIR}/third_party/cgltf
    ${CMAKE_SOURCE_DIR}/third_party/stb
)
