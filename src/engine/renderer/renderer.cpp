#include "renderer.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#ifdef DEBUG_GL
#define CHECK_GL_ERROR(ctx) CheckGLErrors(ctx)
#else
#define CHECK_GL_ERROR(ctx) do {} while(0)
#endif

// --- Shader implementation ---

void Shader::Use() const {
    glUseProgram(m_programHandle);
    CHECK_GL_ERROR("Shader::Use");
}

void Shader::SetUniformMatrix4(const std::string& name, const float* value) const {
    int32_t loc = GetUniformLocation(name);
    if (loc >= 0) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, value);
        CHECK_GL_ERROR("Shader::SetUniformMatrix4");
    }
}

int32_t Shader::GetUniformLocation(const std::string& name) const {
    return glGetUniformLocation(m_programHandle, name.c_str());
}

// --- Renderer implementation ---

bool Renderer::Init(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;

    // Initialize GLEW (required on desktop, but Emscripten handles GL initialization differently)
    // For Emscripten, the WebGL context is already initialized by the browser
#ifndef __EMSCRIPTEN__
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW init failed: " << glewGetErrorString(err) << std::endl;
        return false;
    }
#endif

    // Set up GL state
    glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glViewport(0, 0, m_width, m_height);

    CHECK_GL_ERROR("Renderer::Init - GL state setup");

    // Create debug triangle
    CreateDebugTriangle();

    // Create default shader
    const std::string vertexShader = R"glsl(
        #version 300 es
        precision highp float;

        layout(location = 0) in vec3 position;
        layout(location = 1) in vec3 color;

        out vec3 fragColor;

        uniform mat4 mvpMatrix;

        void main() {
            gl_Position = mvpMatrix * vec4(position, 1.0);
            fragColor = color;
        }
    )glsl";

    const std::string fragmentShader = R"glsl(
        #version 300 es
        precision highp float;

        in vec3 fragColor;
        out vec4 outColor;

        void main() {
            outColor = vec4(fragColor, 1.0);
        }
    )glsl";

    uint32_t programHandle = CompileShaderProgram(vertexShader, fragmentShader);
    if (programHandle == 0) {
        std::cerr << "Failed to compile default shader program" << std::endl;
        return false;
    }

    m_defaultShader = std::make_unique<Shader>(programHandle);
    return true;
}

void Renderer::Shutdown() {
    if (m_debugVAO != 0) {
        glDeleteVertexArrays(1, &m_debugVAO);
        m_debugVAO = 0;
    }
    if (m_debugVBO != 0) {
        glDeleteBuffers(1, &m_debugVBO);
        m_debugVBO = 0;
    }
    m_defaultShader.reset();
}

void Renderer::BeginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CHECK_GL_ERROR("Renderer::BeginFrame");
}

void Renderer::EndFrame() {
    CHECK_GL_ERROR("Renderer::EndFrame");
}

uint32_t Renderer::CreateDebugTriangle() {
    // Vertices: position (3 floats) + color (3 floats)
    float vertices[] = {
        // Position              // Color
        -0.5f, -0.5f, 0.0f,     1.0f, 0.0f, 0.0f,  // Red
         0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,  // Green
         0.0f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f   // Blue
    };

    // Create VAO
    glGenVertexArrays(1, &m_debugVAO);
    glBindVertexArray(m_debugVAO);
    CHECK_GL_ERROR("Renderer::CreateDebugTriangle - GenVertexArrays");

    // Create VBO
    glGenBuffers(1, &m_debugVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_debugVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    CHECK_GL_ERROR("Renderer::CreateDebugTriangle - BufferData");

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    // Color attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    CHECK_GL_ERROR("Renderer::CreateDebugTriangle - VAO setup");

    return 0;  // Single mesh ID for now
}

void Renderer::DrawMesh(uint32_t meshId, Shader* shader, const float* mvpMatrix) {
    if (shader == nullptr) {
        shader = m_defaultShader.get();
    }

    shader->Use();
    shader->SetUniformMatrix4("mvpMatrix", mvpMatrix);

    glBindVertexArray(m_debugVAO);
    glDrawArrays(GL_TRIANGLES, 0, m_debugVertexCount);
    glBindVertexArray(0);

    CHECK_GL_ERROR("Renderer::DrawMesh");
}

uint32_t Renderer::CompileShaderProgram(const std::string& vertexSource,
                                         const std::string& fragmentSource) {
    // Compile vertex shader
    uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vSrc = vertexSource.c_str();
    glShaderSource(vertexShader, 1, &vSrc, nullptr);
    glCompileShader(vertexShader);

    // Check vertex compilation
    int32_t success = 0;
    char infoLog[512] = {0};
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed:\n" << infoLog << std::endl;
        glDeleteShader(vertexShader);
        return 0;
    }

    // Compile fragment shader
    uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fSrc = fragmentSource.c_str();
    glShaderSource(fragmentShader, 1, &fSrc, nullptr);
    glCompileShader(fragmentShader);

    // Check fragment compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed:\n" << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    // Link program
    uint32_t programHandle = glCreateProgram();
    glAttachShader(programHandle, vertexShader);
    glAttachShader(programHandle, fragmentShader);
    glLinkProgram(programHandle);

    // Check linking
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(programHandle, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
        glDeleteProgram(programHandle);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    CHECK_GL_ERROR("Renderer::CompileShaderProgram");
    return programHandle;
}

void Renderer::CheckGLErrors(const std::string& context) {
#ifdef DEBUG_GL
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "[GL ERROR] " << context << ": 0x" << std::hex << err << std::dec << std::endl;
    }
#endif
}
