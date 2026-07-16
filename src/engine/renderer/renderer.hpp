#pragma once

#include <string>
#include <vector>

// Forward declarations
class Shader;

// TODO: Replace raw pointer with std::unique_ptr once allocator custom is implemented
// (Bootstrap phase — will fix in memory management phase)

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
    bool Init(unsigned int width, unsigned int height);

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
    unsigned int CreateDebugTriangle();

    /**
     * Load a glTF/glb model into GPU buffers.
     * 
     * @param modelPath Relative or absolute path to the model file
     * @return mesh ID, or UINT_MAX on failure
     */
    unsigned int LoadModel(const std::string& modelPath);

    /**
     * Draw the mesh with the given shader and transformation.
     * 
     * @param meshId ID of the mesh to draw
     * @param shader Shader program to use
     * @param mvpMatrix Model-view-projection matrix
     */
    void DrawMesh(unsigned int meshId, Shader* shader, const float* mvpMatrix);

    /**
     * Get the current framebuffer dimensions.
     */
    unsigned int GetWidth() const { return m_width; }
    unsigned int GetHeight() const { return m_height; }

    /**
     * Get the current frame time (for stats/debug).
     */
    float GetFrameTime() const { return m_frameTime; }

private:
    unsigned int m_width = 800;
    unsigned int m_height = 600;
    float m_frameTime = 0.0f;

    struct Mesh {
        unsigned int vao = 0;
        unsigned int vbo = 0;
        unsigned int ebo = 0;
        unsigned int indexCount = 0;
        unsigned int textureId = 0;
        bool hasTexture = false;
        bool hasIndices = false;
    };

    // GPU resources (VAO, VBO for debug triangle)
    unsigned int m_debugVAO = 0;
    unsigned int m_debugVBO = 0;
    unsigned int m_debugVertexCount = 3;

    // Loaded model meshes
    std::vector<Mesh> m_meshes;

    // Default shader program (raw pointer for bootstrap — will use unique_ptr with allocator later)
    Shader* m_defaultShader = nullptr;

    /**
     * Compile and link a shader program.
     * 
     * @param vertexSource GLSL vertex shader source code
     * @param fragmentSource GLSL fragment shader source code
     * @return OpenGL program handle, or 0 if compilation failed
     */
    unsigned int CompileShaderProgram(const std::string& vertexSource,
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
    Shader(unsigned int programHandle) : m_programHandle(programHandle) {}

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
    unsigned int GetHandle() const { return m_programHandle; }

private:
    unsigned int m_programHandle = 0;

    /**
     * Get the uniform location by name.
     */
    int GetUniformLocation(const std::string& name) const;
};
