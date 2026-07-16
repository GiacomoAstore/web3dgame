#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// Forward declarations
class Shader;

/**
 * Renderer - Manages WebGL2 context, shaders, VAOs, VBOs, and draw calls.
 * Responsible for all GPU-side rendering. Does not know about game logic.
 */
class Renderer {
public:
    /**
     * Initialize the WebGL2 context and default state.
     * Should be called once at startup (from Engine::Init).
     * 
     * @return true if initialization succeeded, false otherwise
     */
    bool Init(uint32_t width, uint32_t height);

    /**
     * Clean up GPU resources (shaders, buffers, VAOs).
     */
    void Shutdown();

    /**
     * Clear the framebuffer and prepare for a new frame.
     */
    void BeginFrame();

    /**
     * Submit any pending draw calls and present the frame.
     */
    void EndFrame();

    /**
     * Create a simple triangle mesh for debugging/testing.
     * Allocates VAO + VBO internally.
     * 
     * @return handle/ID of the created mesh (currently just returns 0 for single mesh)
     */
    uint32_t CreateDebugTriangle();

    /**
     * Draw the mesh with the given shader and transformation.
     * 
     * @param meshId ID of the mesh to draw
     * @param shader Shader program to use
     * @param mvpMatrix Model-view-projection matrix
     */
    void DrawMesh(uint32_t meshId, Shader* shader, const float* mvpMatrix);

    /**
     * Get the current framebuffer dimensions.
     */
    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }

    /**
     * Get the current frame time (for stats/debug).
     */
    float GetFrameTime() const { return m_frameTime; }

private:
    uint32_t m_width = 800;
    uint32_t m_height = 600;
    float m_frameTime = 0.0f;

    // GPU resources (VAO, VBO for debug triangle)
    uint32_t m_debugVAO = 0;
    uint32_t m_debugVBO = 0;
    uint32_t m_debugVertexCount = 3;

    // Default shader program
    std::unique_ptr<Shader> m_defaultShader;

    /**
     * Compile and link a shader program.
     * 
     * @param vertexSource GLSL vertex shader source code
     * @param fragmentSource GLSL fragment shader source code
     * @return OpenGL program handle, or 0 if compilation failed
     */
    uint32_t CompileShaderProgram(const std::string& vertexSource,
                                   const std::string& fragmentSource);

    /**
     * Check for GL errors and log them.
     * Only active if DEBUG_GL is defined.
     */
    void CheckGLErrors(const std::string& context);
};

/**
 * Shader - Wraps a compiled OpenGL shader program.
 */
class Shader {
public:
    /**
     * Create a shader from vertex and fragment sources.
     */
    Shader(uint32_t programHandle) : m_programHandle(programHandle) {}

    /**
     * Use this shader for subsequent draw calls.
     */
    void Use() const;

    /**
     * Set a uniform matrix4 variable.
     */
    void SetUniformMatrix4(const std::string& name, const float* value) const;

    /**
     * Get the OpenGL program handle (for internal use).
     */
    uint32_t GetHandle() const { return m_programHandle; }

private:
    uint32_t m_programHandle = 0;

    /**
     * Get the uniform location by name.
     */
    int32_t GetUniformLocation(const std::string& name) const;
};
