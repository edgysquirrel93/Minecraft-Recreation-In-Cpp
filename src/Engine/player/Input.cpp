#include "Input.h"
#include "Engine/ui/UIManager.h"

namespace engine::input
{
void Input::processInput(GLFWwindow *window)
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        ui::UIManager::setCurrentScreen(ui::ScreenState::PauseMenuScreen);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}
}
