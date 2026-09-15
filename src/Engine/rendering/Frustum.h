#ifndef MINECRAFT_RECREATION_RECREATION_FRUSTUM_H
#define MINECRAFT_RECREATION_RECREATION_FRUSTUM_H
#pragma once
#include <glm/glm.hpp>
#include <array>

namespace engine::rendering {

struct Plane {
    glm::vec3 normal{0.0f, 1.0f, 0.0f};
    float distance{0.0f};

    void normalize() {
        if (const float length = glm::length(normal); length > 0.0f) {
            normal /= length;
            distance /= length;
        }
    }

    [[nodiscard]] float getSignedDistanceToPlane(const glm::vec3& point) const {
        return glm::dot(normal, point) + distance;
    }
};

struct BoundingBox {
    glm::vec3 min;
    glm::vec3 max;
};

class Frustum {
public:
    enum PlaneID { Left = 0, Right, Bottom, Top, Near, Far };

    void update(const glm::mat4& viewProjection) {
        // Left
        m_Planes[Left].normal.x = viewProjection[0][3] + viewProjection[0][0];
        m_Planes[Left].normal.y = viewProjection[1][3] + viewProjection[1][0];
        m_Planes[Left].normal.z = viewProjection[2][3] + viewProjection[2][0];
        m_Planes[Left].distance = viewProjection[3][3] + viewProjection[3][0];

        // Right
        m_Planes[Right].normal.x = viewProjection[0][3] - viewProjection[0][0];
        m_Planes[Right].normal.y = viewProjection[1][3] - viewProjection[1][0];
        m_Planes[Right].normal.z = viewProjection[2][3] - viewProjection[2][0];
        m_Planes[Right].distance = viewProjection[3][3] - viewProjection[3][0];

        // Bottom
        m_Planes[Bottom].normal.x = viewProjection[0][3] + viewProjection[0][1];
        m_Planes[Bottom].normal.y = viewProjection[1][3] + viewProjection[1][1];
        m_Planes[Bottom].normal.z = viewProjection[2][3] + viewProjection[2][1];
        m_Planes[Bottom].distance = viewProjection[3][3] + viewProjection[3][1];

        // Top
        m_Planes[Top].normal.x = viewProjection[0][3] - viewProjection[0][1];
        m_Planes[Top].normal.y = viewProjection[1][3] - viewProjection[1][1];
        m_Planes[Top].normal.z = viewProjection[2][3] - viewProjection[2][1];
        m_Planes[Top].distance = viewProjection[3][3] - viewProjection[3][1];

        // Near
        m_Planes[Near].normal.x = viewProjection[0][3] + viewProjection[0][2];
        m_Planes[Near].normal.y = viewProjection[1][3] + viewProjection[1][2];
        m_Planes[Near].normal.z = viewProjection[2][3] + viewProjection[2][2];
        m_Planes[Near].distance = viewProjection[3][3] + viewProjection[3][2];

        // Far
        m_Planes[Far].normal.x = viewProjection[0][3] - viewProjection[0][2];
        m_Planes[Far].normal.y = viewProjection[1][3] - viewProjection[1][2];
        m_Planes[Far].normal.z = viewProjection[2][3] - viewProjection[2][2];
        m_Planes[Far].distance = viewProjection[3][3] - viewProjection[3][2];

        for (auto& plane : m_Planes) {
            plane.normalize();
        }
    }

    [[nodiscard]] bool isBoxVisible(const BoundingBox& box) const {
        for (const auto& plane : m_Planes) {
            glm::vec3 positiveVertex = box.min;
            if (plane.normal.x >= 0) positiveVertex.x = box.max.x;
            if (plane.normal.y >= 0) positiveVertex.y = box.max.y;
            if (plane.normal.z >= 0) positiveVertex.z = box.max.z;

            if (plane.getSignedDistanceToPlane(positiveVertex) < 0.0f) {
                return false;
            }
        }
        return true;
    }

private:
    std::array<Plane, 6> m_Planes;
};

}
#endif //MINECRAFT_RECREATION_RECREATION_FRUSTUM_H
