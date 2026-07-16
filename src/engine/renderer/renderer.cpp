#include "renderer.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#endif

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <cstdint>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

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

static std::string GetDirectoryPath(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) {
        return std::string();
    }
    return path.substr(0, pos + 1);
}

static bool ReadFileToVector(const std::string& path, std::vector<uint8_t>& outData) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    if (size <= 0) {
        return false;
    }
    file.seekg(0, std::ios::beg);
    outData.resize(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(outData.data()), size)) {
        outData.clear();
        return false;
    }
    return true;
}

static std::string JoinPath(const std::string& baseDir, const std::string& relativePath) {
    if (relativePath.empty()) {
        return baseDir;
    }
    if (relativePath[0] == '/' || relativePath.find("://") != std::string::npos) {
        return relativePath;
    }
    if (baseDir.empty() || baseDir.back() == '/' || baseDir.back() == '\\') {
        return baseDir + relativePath;
    }
    return baseDir + '/' + relativePath;
}

static cgltf_accessor* FindAccessorByType(const cgltf_primitive* primitive, cgltf_attribute_type attributeType) {
    for (cgltf_size i = 0; i < primitive->attributes_count; ++i) {
        if (primitive->attributes[i].type == attributeType) {
            return primitive->attributes[i].data;
        }
    }
    return nullptr;
}

// --- Renderer implementation ---

static Renderer* g_currentRenderer = nullptr;

extern "C" {
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void SetCanvasSize(int width, int height) {
    if (g_currentRenderer) {
        g_currentRenderer->SetViewportSize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    }
}
}

unsigned int Renderer::LoadModel(const std::string& modelPath) {
    std::string modelDir = GetDirectoryPath(modelPath);

    cgltf_options options{};
    cgltf_data* data = nullptr;
    cgltf_result result = cgltf_parse_file(&options, modelPath.c_str(), &data);
    if (result != cgltf_result_success) {
        std::cerr << "Failed to parse glTF file: " << modelPath << std::endl;
        return UINT_MAX;
    }

    result = cgltf_load_buffers(&options, data, modelPath.c_str());
    if (result != cgltf_result_success) {
        std::cerr << "Failed to load glTF buffers: " << modelPath << std::endl;
        cgltf_free(data);
        return UINT_MAX;
    }

    if (data->meshes_count == 0) {
        std::cerr << "No mesh found in glTF: " << modelPath << std::endl;
        cgltf_free(data);
        return UINT_MAX;
    }

    const cgltf_mesh* mesh = &data->meshes[0];
    if (mesh->primitives_count == 0) {
        std::cerr << "No primitive found in glTF mesh: " << modelPath << std::endl;
        cgltf_free(data);
        return UINT_MAX;
    }

    const cgltf_primitive* primitive = &mesh->primitives[0];
    if (primitive->type != cgltf_primitive_type_triangles) {
        std::cerr << "Unsupported primitive type in glTF: " << modelPath << std::endl;
        cgltf_free(data);
        return UINT_MAX;
    }

    cgltf_accessor* positionAccessor = FindAccessorByType(primitive, cgltf_attribute_type_position);
    if (!positionAccessor) {
        std::cerr << "Missing POSITION attribute in glTF mesh: " << modelPath << std::endl;
        cgltf_free(data);
        return UINT_MAX;
    }

    cgltf_accessor* texcoordAccessor = FindAccessorByType(primitive, cgltf_attribute_type_texcoord);
    cgltf_size vertexCount = positionAccessor->count;
    std::vector<float> positions(static_cast<size_t>(vertexCount) * 3);
    if (cgltf_accessor_unpack_floats(positionAccessor, positions.data(), vertexCount * 3) != vertexCount * 3) {
        std::cerr << "Failed to unpack POSITION data for glTF mesh: " << modelPath << std::endl;
        cgltf_free(data);
        return UINT_MAX;
    }

    std::vector<float> texcoords;
    if (texcoordAccessor) {
        texcoords.resize(static_cast<size_t>(vertexCount) * 2);
        if (cgltf_accessor_unpack_floats(texcoordAccessor, texcoords.data(), vertexCount * 2) != vertexCount * 2) {
            std::cerr << "Failed to unpack TEXCOORD_0 data for glTF mesh: " << modelPath << std::endl;
            cgltf_free(data);
            return UINT_MAX;
        }
    } else {
        texcoords.resize(static_cast<size_t>(vertexCount) * 2);
        std::fill(texcoords.begin(), texcoords.end(), 0.0f);
    }

    std::vector<float> vertices;
    vertices.resize(static_cast<size_t>(vertexCount) * 5);
    for (size_t i = 0; i < static_cast<size_t>(vertexCount); ++i) {
        vertices[i * 5 + 0] = positions[i * 3 + 0];
        vertices[i * 5 + 1] = positions[i * 3 + 1];
        vertices[i * 5 + 2] = positions[i * 3 + 2];
        vertices[i * 5 + 3] = texcoords[i * 2 + 0];
        vertices[i * 5 + 4] = texcoords[i * 2 + 1];
    }

    std::vector<uint32_t> indices;
    bool hasIndices = false;
    if (primitive->indices) {
        cgltf_size indexCount = primitive->indices->count;
        indices.resize(static_cast<size_t>(indexCount));
        if (cgltf_accessor_unpack_indices(primitive->indices, indices.data(), sizeof(uint32_t), indexCount) != indexCount) {
            std::cerr << "Failed to unpack indices for glTF mesh: " << modelPath << std::endl;
            cgltf_free(data);
            return UINT_MAX;
        }
        hasIndices = true;
    }

    unsigned int textureId = 0;
    bool hasTexture = false;
    if (primitive->material && primitive->material->has_pbr_metallic_roughness) {
        const cgltf_texture* texture = primitive->material->pbr_metallic_roughness.base_color_texture.texture;
        if (texture && texture->image) {
            const cgltf_image* image = texture->image;
            std::vector<uint8_t> imageData;
            int width = 0, height = 0, channels = 0;
            unsigned char* pixels = nullptr;
            if (image->uri) {
                std::string imagePath = JoinPath(modelDir, image->uri);
                if (ReadFileToVector(imagePath, imageData)) {
                    pixels = stbi_load_from_memory(imageData.data(), static_cast<int>(imageData.size()), &width, &height, &channels, STBI_rgb_alpha);
                }
            } else if (image->buffer_view) {
                const cgltf_buffer_view* bufferView = image->buffer_view;
                const uint8_t* bufferData = reinterpret_cast<const uint8_t*>(bufferView->buffer->data) + bufferView->offset;
                pixels = stbi_load_from_memory(bufferData, static_cast<int>(bufferView->size), &width, &height, &channels, STBI_rgb_alpha);
            }

            if (pixels) {
                glGenTextures(1, &textureId);
                glBindTexture(GL_TEXTURE_2D, textureId);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
                glGenerateMipmap(GL_TEXTURE_2D);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glBindTexture(GL_TEXTURE_2D, 0);
                stbi_image_free(pixels);
                hasTexture = true;
            } else {
                std::cerr << "Failed to load texture for glTF model: " << modelPath << std::endl;
            }
        }
    }

    Mesh meshEntry;
    meshEntry.hasTexture = hasTexture;
    meshEntry.textureId = textureId;
    meshEntry.hasIndices = hasIndices;
    meshEntry.indexCount = hasIndices ? static_cast<unsigned int>(indices.size()) : static_cast<unsigned int>(vertexCount);

    glGenVertexArrays(1, &meshEntry.vao);
    glBindVertexArray(meshEntry.vao);

    glGenBuffers(1, &meshEntry.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, meshEntry.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

    if (hasIndices) {
        glGenBuffers(1, &meshEntry.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshEntry.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (hasIndices) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    m_meshes.push_back(meshEntry);
    unsigned int meshId = static_cast<unsigned int>(m_meshes.size() - 1);

    cgltf_free(data);
    return meshId;
}

bool Renderer::Init(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;

#ifdef __EMSCRIPTEN__
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.alpha = false;
    attrs.depth = true;
    attrs.stencil = false;
    attrs.antialias = true;
    attrs.preserveDrawingBuffer = false;
    attrs.enableExtensionsByDefault = true;
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
    if (context <= 0) {
        std::cerr << "Failed to create WebGL2 context" << std::endl;
        return false;
    }

    if (emscripten_webgl_make_context_current(context) != EMSCRIPTEN_RESULT_SUCCESS) {
        std::cerr << "Failed to make WebGL2 context current" << std::endl;
        return false;
    }

    int canvasWidth = 0;
    int canvasHeight = 0;
    if (emscripten_get_canvas_element_size("#canvas", &canvasWidth, &canvasHeight) == EMSCRIPTEN_RESULT_SUCCESS) {
        if (canvasWidth > 0 && canvasHeight > 0) {
            m_width = static_cast<uint32_t>(canvasWidth);
            m_height = static_cast<uint32_t>(canvasHeight);
        }
    }
#else
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW init failed: " << glewGetErrorString(err) << std::endl;
        return false;
    }
#endif

    glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glViewport(0, 0, m_width, m_height);

    g_currentRenderer = this;

    CHECK_GL_ERROR("Renderer::Init - GL state setup");

    unsigned int meshId = LoadModel("assets/models/vehicle-truck-yellow.glb");
    if (meshId == UINT_MAX) {
        std::cerr << "Falling back to debug triangle" << std::endl;
        CreateDebugTriangle();
    }

    const std::string vertexShader = R"glsl(#version 300 es
        precision highp float;

        layout(location = 0) in vec3 position;
        layout(location = 1) in vec2 texcoord;

        out vec2 fragUV;

        uniform mat4 mvpMatrix;

        void main() {
            gl_Position = mvpMatrix * vec4(position, 1.0);
            fragUV = texcoord;
        }
    )glsl";

    const std::string fragmentShader = R"glsl(#version 300 es
        precision highp float;

        in vec2 fragUV;
        uniform sampler2D uTexture;
        uniform bool uUseTexture;
        out vec4 outColor;

        void main() {
            if (uUseTexture) {
                outColor = texture(uTexture, fragUV);
            } else {
                outColor = vec4(1.0, 1.0, 1.0, 1.0);
            }
        }
    )glsl";

    uint32_t programHandle = CompileShaderProgram(vertexShader, fragmentShader);
    if (programHandle == 0) {
        std::cerr << "Failed to compile default shader program" << std::endl;
        return false;
    }

    m_defaultShader = new Shader(programHandle);
    return true;
}

void Renderer::Shutdown() {
    g_currentRenderer = nullptr;
    if (m_debugVAO != 0) {
        glDeleteVertexArrays(1, &m_debugVAO);
        m_debugVAO = 0;
    }
    if (m_debugVBO != 0) {
        glDeleteBuffers(1, &m_debugVBO);
        m_debugVBO = 0;
    }
    if (m_defaultShader) {
        delete m_defaultShader;
        m_defaultShader = nullptr;
    }
}

void Renderer::BeginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CHECK_GL_ERROR("Renderer::BeginFrame");
}

void Renderer::SetViewportSize(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) {
        return;
    }
    m_width = width;
    m_height = height;
    glViewport(0, 0, m_width, m_height);
}

void Renderer::EndFrame() {
    CHECK_GL_ERROR("Renderer::EndFrame");
}

uint32_t Renderer::CreateDebugTriangle() {
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
         0.0f,  0.5f, 0.0f,  0.5f, 1.0f,
    };

    glGenVertexArrays(1, &m_debugVAO);
    glBindVertexArray(m_debugVAO);
    CHECK_GL_ERROR("Renderer::CreateDebugTriangle - GenVertexArrays");

    glGenBuffers(1, &m_debugVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_debugVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    CHECK_GL_ERROR("Renderer::CreateDebugTriangle - BufferData");

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    CHECK_GL_ERROR("Renderer::CreateDebugTriangle - VAO setup");
    return 0;
}

void Renderer::DrawMesh(uint32_t meshId, Shader* shader, const float* mvpMatrix) {
    if (shader == nullptr) {
        shader = m_defaultShader;
    }

    shader->Use();
    shader->SetUniformMatrix4("mvpMatrix", mvpMatrix);

    bool drewMesh = false;
    if (meshId < m_meshes.size()) {
        const Mesh& mesh = m_meshes[meshId];
        GLint uUseTexture = glGetUniformLocation(shader->GetHandle(), "uUseTexture");
        if (uUseTexture >= 0) {
            glUniform1i(uUseTexture, mesh.hasTexture ? 1 : 0);
        }

        if (mesh.hasTexture) {
            GLint uTexture = glGetUniformLocation(shader->GetHandle(), "uTexture");
            if (uTexture >= 0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mesh.textureId);
                glUniform1i(uTexture, 0);
            }
        }

        glBindVertexArray(mesh.vao);
        if (mesh.hasIndices) {
            glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, mesh.indexCount);
        }
        glBindVertexArray(0);
        if (mesh.hasTexture) {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        drewMesh = true;
    }

    if (!drewMesh && m_debugVAO != 0) {
        GLint uUseTexture = glGetUniformLocation(shader->GetHandle(), "uUseTexture");
        if (uUseTexture >= 0) {
            glUniform1i(uUseTexture, 0);
        }
        glBindVertexArray(m_debugVAO);
        glDrawArrays(GL_TRIANGLES, 0, m_debugVertexCount);
        glBindVertexArray(0);
    }

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
    (void)context;
#ifdef DEBUG_GL
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "[GL ERROR] " << context << ": 0x" << std::hex << err << std::dec << std::endl;
    }
#endif
}
