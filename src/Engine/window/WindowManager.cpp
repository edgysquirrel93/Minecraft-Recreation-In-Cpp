#include "GLAD/include/glad/glad.h"

#include "WindowManager.h"
#include <iostream>

namespace engine::window {

void WindowManager::initWindow() {
    if (!glfwInit()) return;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor {glfwGetPrimaryMonitor()};
    // const GLFWvidmode* mode {glfwGetVideoMode(monitor)};

    int monitorX, monitorY, monitorWidth, monitorHeight;
    glfwGetMonitorWorkarea(monitor, &monitorX, &monitorY, &monitorWidth, &monitorHeight);

    m_Window = glfwCreateWindow(monitorWidth, monitorHeight, "Minecraft Recreation", nullptr, nullptr);

    if (!m_Window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }

    glfwMakeContextCurrent(m_Window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cout << "Failed to initialize GLAD!" << std::endl;
    }
}

} // engine::window
