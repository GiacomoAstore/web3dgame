add_library(game STATIC
    game.cpp
)

target_include_directories(game PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/engine
)

target_link_libraries(game
    engine
)
