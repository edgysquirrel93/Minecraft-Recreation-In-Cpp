#include "Input.h"

#include <iostream>

#include "Engine/ui/UIManager.h"
#include "Engine/config/SettingsManager.h"

namespace engine::input
{

void Input::processInput(GLFWwindow *window)
{
    if (ui::UIManager::getCurrentScreen() == ui::ScreenState::InGame) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        ui::UIManager::setCurrentScreen(ui::ScreenState::PauseMenuScreen);
        ui::UIManager::disableOverlay(ui::DebugScreen);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    static bool f3Pressed {false};
    const bool f3Down {(glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS)};

    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS && !f3Pressed)
    {
        f3Pressed = true;
        ui::UIManager::toggleOverlay(ui::DebugScreen);

        if (ui::UIManager::isOverlayActive(ui::DebugScreen)) {
            glfwSwapInterval(0);
        } else {
            glfwSwapInterval(1);
        }

    }  else if (!f3Down) {
        f3Pressed = false;
    }
}
}

void Input::enterGameInputMode(GLFWwindow* window)
{
    ui::UIManager::setCurrentScreen(ui::ScreenState::InGame);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    Camera::resetMouseFlag();
}

void Camera::mouseCallback(GLFWwindow* /*window*/, const double xposIn, const double yposIn) {
    if (ui::UIManager::getCurrentScreen() != ui::ScreenState::InGame) {
        return;
    }

    const auto xpos {static_cast<float>(xposIn)};
    const auto ypos {static_cast<float>(yposIn)};

    if (m_FirstMouse) {
        m_LastX = xpos;
        m_LastY = ypos;
        m_FirstMouse = false;
    }

    float xoffset {xpos - m_LastX};
    float yoffset {m_LastY - ypos};
    m_LastX = xpos;
    m_LastY = ypos;

    const float finalSensitivity = config::SettingsManager::get().getSensitivity() * 0.15f;
    xoffset *= finalSensitivity;
    yoffset *= finalSensitivity;

    m_Yaw += xoffset;
    m_Pitch += yoffset;

    if (m_Pitch > 89.0f)  m_Pitch = 89.0f;
    if (m_Pitch < -89.0f) m_Pitch = -89.0f;

    glm::vec3 front;
    front.x = static_cast<float>(cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch)));
    front.y = static_cast<float>(sin(glm::radians(m_Pitch)));
    front.z = static_cast<float>(sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch)));

    m_Front = glm::normalize(front);
}
}
