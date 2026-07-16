#include "game.hpp"

#include "../engine/renderer/renderer.hpp"
#include "../engine/math/math.hpp"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

bool Game::Init(Renderer* renderer) {
    m_renderer = renderer;
    m_elapsedTime = 0.0f;

    // Load track model
    // Note: The renderer auto-loads vehicle-truck-yellow.glb as meshId 0 in Renderer::Init()
    // We load the track as a second model, which gets meshId 1
    m_vehicleMeshId = 0;  // First mesh loaded by Renderer::Init
    m_trackMeshId = renderer->LoadModel("assets/models/track-straight.glb");
    
    if (m_trackMeshId != UINT_MAX) {
        std::cout << "[Game] Track loaded successfully (meshId=" << m_trackMeshId << ")" << std::endl;
    } else {
        std::cout << "[Game] Failed to load track, using fallback" << std::endl;
    }

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
    // Simple perspective camera positioned to see both track and vehicle
    float aspect = (float)renderer->GetWidth() / (float)renderer->GetHeight();
    Mat4 projection = Math::Perspective(45.0f, aspect, 0.1f, 100.0f);
    
    // Position camera to view the scene from slightly elevated angle
    Mat4 view = Math::LookAt(
        Vec3(3.0f, 2.5f, 3.0f),   // Camera position (elevated and offset)
        Vec3(0.0f, 0.0f, 0.0f),   // Look at origin (center of scene)
        Vec3(0.0f, 1.0f, 0.0f)    // Up
    );

    // Draw the track at origin
    if (m_trackMeshId != UINT_MAX) {
        Mat4 trackModel = Math::Translate(glm::vec3(0.0f, 0.0f, 0.0f));
        Mat4 trackMVP = projection * view * trackModel;
        renderer->DrawMesh(m_trackMeshId, nullptr, glm::value_ptr(trackMVP));
    }

    // Draw the vehicle moving and rotating on the track
    if (m_vehicleMeshId != UINT_MAX) {
        // Simple vehicle animation: move along X axis and rotate
        float vehicleX = std::sin(m_elapsedTime * 0.5f) * 2.0f;  // Side-to-side motion
        float vehicleRotation = m_elapsedTime * 0.8f;             // Continuous rotation
        
        // Build vehicle transformation: translate to track position, then rotate
        Mat4 vehicleModel = Math::Translate(glm::vec3(vehicleX, 0.5f, 0.0f))  // Position on track
                            * Math::Rotate(glm::quat(glm::vec3(0.0f, vehicleRotation, 0.0f)));  // Rotate around Y
        
        Mat4 vehicleMVP = projection * view * vehicleModel;
        renderer->DrawMesh(m_vehicleMeshId, nullptr, glm::value_ptr(vehicleMVP));
    }
}
