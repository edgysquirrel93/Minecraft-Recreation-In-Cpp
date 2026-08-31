#ifndef MINECRAFT_RECREATION_RECREATION_INPUT_H
#define MINECRAFT_RECREATION_RECREATION_INPUT_H
#include "GLFW/glfw3.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Engine/worldgen/ChunkRendering.h"

namespace engine::input
{
class Input {
public:
    static void processInput(GLFWwindow* window);
    static void enterGameInputMode(GLFWwindow* window);
    static bool grounded(const glm::vec3& pos);
    // static bool checkCollision(const glm::vec3& pos);
};

class Player {
    static inline float s_TargetFov {};
    static inline float s_VerticalVelocity {};
    static inline float s_CoyoteTime {};
    static inline float s_JumpForce {8.4f};
    static inline float s_Gravity {-32.0f};
    static inline bool s_IsGrounded {};
    static inline float s_CoyoteDuration {0.1f};
    static inline bool s_SpaceWasPressed {false};
    static inline bool s_IsFlying {false};
    static inline float s_LastSpaceTime {0.0f};
    static inline bool s_LeftMousePressed {false};
    static inline bool s_RightMousePressed {false};
    static inline uint8_t s_BuildingBlock;
    static void processSurvivalMovement(GLFWwindow* window, float deltaTime);
    static void processCreativeMovement(GLFWwindow* window, float deltaTime);
    static void processGravity(float deltaTime);
public:

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
