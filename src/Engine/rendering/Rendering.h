#ifndef MINECRAFT_RECREATION_RECREATION_RENDERING_H
#define MINECRAFT_RECREATION_RECREATION_RENDERING_H
#include <filesystem>

#include "Engine/shaders/shaders.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <unordered_map>

namespace engine::rendering
{

class ShaderManager {
    std::unordered_map<std::string, std::unique_ptr<shaders::Shader>> m_Shaders;
public:
    ShaderManager() {
        load("main", "assets/Shaders/Vertices/MainShader.vs",
                         "assets/Shaders/Fragments/MainShader.fs");
        load("crosshair", "assets/Shaders/Vertices/CrosshairShader.vs",
            "assets/Shaders/Fragments/CrosshairShader.fs");
    }

    void load(const std::string& name, const std::string& vsPath, const std::string& fsPath) {
        m_Shaders[name] = std::make_unique<shaders::Shader>(vsPath.c_str(), fsPath.c_str());
    }

    shaders::Shader* get(const std::string& name) {
        const auto it = m_Shaders.find(name);
        return (it != m_Shaders.end()) ? it->second.get() : nullptr;
    }
};

class Rendering {
    glm::mat4 m_View;
public:
    void gameRender(ShaderManager& shaderManager, GLFWwindow* window);

    [[nodiscard]] glm::mat4 getViewMatrix() const {return m_View;}
    void setViewMatrix(const glm::mat4& v) {m_View = v;}
};

} // engine::rendering

#endif
