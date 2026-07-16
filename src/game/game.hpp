#pragma once

// Forward declarations
class Renderer;

/**
 * Game - Application-layer game logic.
 * Does not know about rendering internals (only receives Renderer* in Render()).
 * Orchestrates game entities, physics, state, and high-level behavior.
 */
class Game {
public:
    /**
     * Initialize game state and systems.
     * Called once at startup.
     * 
     * @param renderer Renderer instance (for initialization of game-level rendering resources if needed)
     * @return true if init succeeded
     */
    bool Init(Renderer* renderer);

    /**
     * Shutdown game and release resources.
     */
    void Shutdown();

    /**
     * Update game logic for one frame.
     * Called from Engine::Tick before rendering.
     * 
     * @param deltaTime Time elapsed since last frame, in seconds
     */
    void Update(float deltaTime);

    /**
     * Render the current game state.
     * Called from Engine::Tick after update.
     * Uses the provided Renderer to draw.
     * 
     * @param renderer Renderer instance to use for drawing
     */
    void Render(Renderer* renderer);

private:
    Renderer* m_renderer = nullptr;
    float m_elapsedTime = 0.0f;
    
    // Loaded model mesh IDs
    unsigned int m_vehicleMeshId = 0xFFFFFFFF;  // vehicle-truck-yellow.glb
    unsigned int m_trackMeshId = 0xFFFFFFFF;    // track-straight.glb
};
