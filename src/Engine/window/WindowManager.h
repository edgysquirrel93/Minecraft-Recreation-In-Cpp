#ifndef MINECRAFT_RECREATION_RECREATION_WINDOWMANAGER_H
#define MINECRAFT_RECREATION_RECREATION_WINDOWMANAGER_H

#include <GLFW/glfw3.h>

namespace engine::window {

class WindowManager {
    GLFWwindow* m_Window {nullptr};
    bool m_LastFullscreenState {};
public:
    [[nodiscard]] GLFWwindow* getWindow() const {return m_Window;}

    static void framebufferSizeCallback(GLFWwindow* /*window*/, const int width, int height) {
        if (height == 0) height = 1; glViewport(0, 0, width, height);}

    [[nodiscard]] int getWindowHeight() const {int width, height; glfwGetWindowSize(getWindow(), &width, &height); return height;}
    [[nodiscard]] int getWindowWidth() const {int width, height; glfwGetWindowSize(getWindow(), &width, &height); return width;}

    void initWindow();
    void checkWindowState();
};

} // namespace::window

#endif
