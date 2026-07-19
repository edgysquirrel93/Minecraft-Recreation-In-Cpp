#include "glad/gl.h"

#include "WindowManager.h"
#include <iostream>

#include "Engine/config/SettingsManager.h"

namespace engine::window {

void WindowManager::initWindow() {
    if (!glfwInit()) return;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor {glfwGetPrimaryMonitor()};

    int monitorX, monitorY, monitorWidth, monitorHeight;
    glfwGetMonitorWorkarea(monitor, &monitorX, &monitorY, &monitorWidth, &monitorHeight);

    m_Window = glfwCreateWindow(monitorWidth, monitorHeight, "Minecraft Recreation", nullptr, nullptr);

    if (!m_Window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }

    glfwMakeContextCurrent(m_Window);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD!" << std::endl;
    }
}

// Fullscreening
void WindowManager::checkWindowState() {
    GLFWmonitor* monitor {glfwGetPrimaryMonitor()};
    const GLFWvidmode* mode {glfwGetVideoMode(monitor)};

    if (config::SettingsManager::get().getFullscreenBool() == m_LastFullscreenState) return;

    int monitorX, monitorY, monitorWidth, monitorHeight;
    glfwGetMonitorWorkarea(monitor, &monitorX, &monitorY, &monitorWidth, &monitorHeight);
    if (config::SettingsManager::get().getFullscreenBool() != m_LastFullscreenState) {
        if (config::SettingsManager::get().getFullscreenBool()) {
            glfwSetWindowMonitor(m_Window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        } else {
            glfwSetWindowMonitor(m_Window, nullptr, monitorX, monitorY, monitorWidth, monitorHeight, 0);
            glfwMaximizeWindow(m_Window);
        }
        m_LastFullscreenState = config::SettingsManager::get().getFullscreenBool();
    }
}

} // engine::window
