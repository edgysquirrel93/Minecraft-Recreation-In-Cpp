#ifndef MINECRAFT_RECREATION_RECREATION_INPUT_H
#define MINECRAFT_RECREATION_RECREATION_INPUT_H
#include "GLFW/glfw3.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Engine/worldgen/ChunkRendering.h"

namespace engine::input
{
class Input {
public:
    // void updatePlayer(float deltaTime);
    static void processInput(GLFWwindow* window);
    static void enterGameInputMode(GLFWwindow* window);
};

class Player {
    static inline float s_TargetFov {};
    static inline float s_VerticalVelocity {};
    static inline float s_CoyoteTime {};
    static inline float s_JumpForce {};
    static inline float s_Gravity {};
    static inline float s_CoyoteDuration {};
    static inline bool s_SpaceWasPressed {false};
    static inline bool s_IsFlying {false};
    static inline float s_LastSpaceTime {0.0f};
    static inline bool s_LeftMousePressed {false};
    static void processSurvivalMovement(GLFWwindow* window, float deltaTime);
    static void processCreativeMovement(GLFWwindow* window, float deltaTime);

    static inline worldgen::ChunkRendering* s_ActiveChunk {nullptr};
public:

    static void init(worldgen::ChunkRendering* chunk) { s_ActiveChunk = chunk; }

    static void processMovement(GLFWwindow* window, float deltaTime);

    [[nodiscard]] static float getTargetFov() {return s_TargetFov;}
};

class Camera {

    static inline glm::vec3 m_CameraFront;

    static inline bool m_FirstMouse {true};

    static inline float m_Yaw   {-90.0f};
    static inline float m_Pitch {0.0f};
    static inline float m_LastX {};
    static inline float m_LastY {};

public:

    struct RaycastResult {
        bool hit;
        glm::ivec3 blockPos{};
        glm::ivec3 placePos{0};
    };

    static void mouseCallback(GLFWwindow* /*window*/, double xposIn, double yposIn);
    static RaycastResult raycast(glm::vec3 start, glm::vec3 dir, float maxDist, worldgen::ChunkRendering& chunk);

    static glm::vec3 getCameraFront() {return m_CameraFront;}
    static void resetMouseFlag() {m_FirstMouse = false;}
};
}

#endif //MINECRAFT_RECREATION_RECREATION_INPUT_H
