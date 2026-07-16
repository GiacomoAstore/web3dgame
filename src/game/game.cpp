#include "game.hpp"

#include "../engine/renderer/renderer.hpp"
#include "../engine/math/math.hpp"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

bool Game::Init(Renderer* renderer) {
    m_renderer = renderer;
    m_elapsedTime = 0.0f;

    std::cout << "[Game] Initialized" << std::endl;
    return true;
}

void Game::Shutdown() {
    std::cout << "[Game] Shutdown" << std::endl;
}

void Game::Update(float deltaTime) {
    m_elapsedTime += deltaTime;
    // TODO: game logic here (input, physics, entities, etc.)
}

void Game::Render(Renderer* renderer) {
    // Simple perspective camera
    float aspect = (float)renderer->GetWidth() / (float)renderer->GetHeight();
    Mat4 projection = Math::Perspective(45.0f, aspect, 0.1f, 100.0f);
    Mat4 view = Math::LookAt(
        Vec3(0.0f, 0.0f, 2.0f),   // Camera position
        Vec3(0.0f, 0.0f, 0.0f),   // Look at
        Vec3(0.0f, 1.0f, 0.0f)    // Up
    );

    // Slow rotation of the triangle
    float angle = m_elapsedTime * 0.5f;
    Mat4 model = Math::Rotate(glm::quat(glm::vec3(0.0f, angle, 0.0f)));

    // Combine transformations
    Mat4 mvp = projection * view * model;

    // Draw the debug triangle
    renderer->DrawMesh(0, nullptr, glm::value_ptr(mvp));
}
