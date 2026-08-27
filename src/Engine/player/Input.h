#ifndef MINECRAFT_RECREATION_RECREATION_INPUT_H
#define MINECRAFT_RECREATION_RECREATION_INPUT_H
#include "GLFW/glfw3.h"
#include <glm/gtc/matrix_transform.hpp>

namespace engine::input
{
class Input {
public:
    // void updatePlayer(float deltaTime);
    static void processInput(GLFWwindow* window);
    static void enterGameInputMode(GLFWwindow* window);
};
class Camera {

    static inline glm::vec3 m_Front;

    static inline bool m_FirstMouse {true};

    static inline float m_Yaw   {-90.0f};
    static inline float m_Pitch {0.0f};
    static inline float m_LastX {};
    static inline float m_LastY {};

public:
    static void mouseCallback(GLFWwindow* /*window*/, double xposIn, double yposIn);

    static glm::vec3 getFront() {return m_Front;}
    static void resetMouseFlag() {m_FirstMouse = false;}
};
}

#endif //MINECRAFT_RECREATION_RECREATION_INPUT_H
