#pragma once

// TODO: Replace raw pointers with std::unique_ptr once allocator custom is implemented
// (See REGOLE.md §2 and REGOLE.md §6 — currently using raw pointers due to Emscripten
// toolchain constraints in this bootstrap phase)

// Forward declarations
class Renderer;
class Game;

/**
 * Engine - Core bootstrap and game loop.
 * Owns Renderer and delegates to Game for logic updates.
 * Responsible for: initialization, frame timing, and main loop orchestration.
 */
class Engine {
public:
    /**
     * Initialize the engine, renderer, and game systems.
     * Call once before the main loop.
     * 
     * @return true if all systems initialized successfully
     */
    bool Init(unsigned int windowWidth, unsigned int windowHeight);

    /**
     * Shutdown all engine and game systems.
     * Call before exit.
     */
    void Shutdown();

    /**
     * Main frame tick: updates game logic and renders one frame.
     * Call this repeatedly from the Emscripten main loop.
     */
    void Tick();

    /**
     * Check if the engine is still running (not requested to shut down).
     */
    bool IsRunning() const { return m_isRunning; }

    /**
     * Get the renderer (for debugging or direct access if needed).
     */
    Renderer* GetRenderer() { return m_renderer; }

    /**
     * Get the game instance.
     */
    Game* GetGame() { return m_game; }

    /**
     * Request graceful shutdown on next Tick.
     */
    void RequestShutdown() { m_isRunning = false; }

private:
    bool m_isRunning = true;

    // Engine systems (raw pointers for bootstrap — will use unique_ptr with custom allocator later)
    Renderer* m_renderer = nullptr;
    Game* m_game = nullptr;

    // Frame timing
    float m_deltaTime = 0.016f;  // ~60 fps
    float m_currentTime = 0.0f;
    float m_lastFrameTime = 0.0f;
};

/**
 * Global engine instance (used by Emscripten callback and JS bindings).
 */
extern Engine* g_engine;
