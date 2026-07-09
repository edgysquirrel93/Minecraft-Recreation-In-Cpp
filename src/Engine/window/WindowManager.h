#ifndef MINCRAFT_RECREATION_RECREATION_WINDOWMANAGER_H
#define MINCRAFT_RECREATION_RECREATION_WINDOWMANAGER_H

#include <GLFW/include/GLFW/glfw3.h>

namespace engine::window {

class WindowManager {
    GLFWwindow* m_Window {nullptr};
public:
    [[nodiscard]] GLFWwindow* getWindow() const {return m_Window;}
    void initWindow();
};

} // namespace::window

#endif
