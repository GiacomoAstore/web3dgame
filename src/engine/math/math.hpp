#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Re-export GLM for convenient use across the engine
// This header acts as the single point of math types for the project

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat4 = glm::mat4;
using Quat = glm::quat;

namespace Math {
    // Identity matrix
    inline Mat4 Identity() {
        return glm::identity<Mat4>();
    }

    // Perspective projection
    inline Mat4 Perspective(float fovy, float aspect, float near, float far) {
        return glm::perspective(glm::radians(fovy), aspect, near, far);
    }

    // Orthographic projection
    inline Mat4 Orthographic(float left, float right, float bottom, float top, float near, float far) {
        return glm::ortho(left, right, bottom, top, near, far);
    }

    // Look at view matrix
    inline Mat4 LookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
        return glm::lookAt(eye, center, up);
    }

    // Translation matrix
    inline Mat4 Translate(const Vec3& pos) {
        return glm::translate(glm::identity<Mat4>(), pos);
    }

    // Rotation matrix from quaternion
    inline Mat4 Rotate(const Quat& q) {
        return glm::mat4_cast(q);
    }

    // Scale matrix
    inline Mat4 Scale(const Vec3& s) {
        return glm::scale(glm::identity<Mat4>(), s);
    }

    // Vector magnitude
    inline float Length(const Vec3& v) {
        return glm::length(v);
    }

    // Normalized vector
    inline Vec3 Normalize(const Vec3& v) {
        return glm::normalize(v);
    }

    // Dot product
    inline float Dot(const Vec3& a, const Vec3& b) {
        return glm::dot(a, b);
    }

    // Cross product
    inline Vec3 Cross(const Vec3& a, const Vec3& b) {
        return glm::cross(a, b);
    }
}
