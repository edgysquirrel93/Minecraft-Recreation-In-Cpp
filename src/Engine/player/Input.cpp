#include "Input.h"

#include <glm/gtx/norm.hpp>

#include "Engine/ui/UIManager.h"
#include "Engine/config/SettingsManager.h"

namespace engine::input
{

void Input::processInput(GLFWwindow *window)
{
    if (ui::UIManager::getCurrentScreen() == ui::ScreenState::InGame) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        ui::UIManager::setCurrentScreen(ui::ScreenState::PauseMenuScreen);
        ui::UIManager::setLastScreen(ui::ScreenState::PauseMenuScreen);
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

void Player::processSurvivalMovement(GLFWwindow* window, const float deltaTime) {
    if (!s_ActiveChunk) return;

    glm::vec3 moveDir {};
    float speed {4.317f * deltaTime};
    const glm::vec3 cameraFront = Camera::getCameraFront();

    auto flatFront = glm::vec3(cameraFront.x, 0.0f, cameraFront.z);
    if (glm::length(flatFront) > 0.0001f) {
        flatFront = glm::normalize(flatFront);
    } else {
        flatFront = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    const glm::vec3 flatRight {glm::normalize(glm::cross(flatFront, glm::vec3(0.0f, 1.0f, 0.0f)))};

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += flatFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= flatFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= flatRight;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += flatRight;

    glm::vec3 cameraPos {config::LevelData::get().getCameraPos()};

    if (glm::length(moveDir) > 0.0f) {
        moveDir = glm::normalize(moveDir);

        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            speed *= 2.0f;
            s_TargetFov = config::SettingsManager::get().getBaseFov() * 1.25f;
        }

        glm::vec3 newPos {cameraPos + moveDir * speed};

            cameraPos.x = newPos.x;

            cameraPos.z = newPos.z;

        config::LevelData::get().setCameraPos(cameraPos);
    } else {
        s_TargetFov = config::SettingsManager::get().getBaseFov();
    }

    float currentFov {config::SettingsManager::get().getBaseFov()};

    if (deltaTime > 0.0f && deltaTime < 0.1f) {
        currentFov += (s_TargetFov - currentFov) * 10.0f * deltaTime;
    } else if (deltaTime == 0.0f) {
        currentFov = s_TargetFov;
    }

    if (currentFov < 30.0f) currentFov = 30.0f;
    if (currentFov > 150.0f) currentFov = 150.0f;

    auto [hit, blockPos, placePos] {Camera::raycast(config::LevelData::get().getCameraPos(),
Camera::getCameraFront(), 5.0f, *s_ActiveChunk)};


    bool currentLeft = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (currentLeft && !s_LeftMousePressed) {
        if (hit) {
            std::cout << "Hit block at: " << blockPos.x << ", " << blockPos.y << ", " << blockPos.z << "\n";
            s_ActiveChunk->setBlock(blockPos.x, blockPos.y, blockPos.z, blockregistry::AIR);
        } else {
            std::cout << "Raycast missed or out of range\n";
        }
    }
    s_LeftMousePressed = currentLeft;
}

void Player::processCreativeMovement(GLFWwindow* window, const float deltaTime) {
    processSurvivalMovement(window, deltaTime);

    const bool currentSpace {glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS};
    glm::vec3 cameraPos {config::LevelData::get().getCameraPos()};

    if (currentSpace && !s_SpaceWasPressed) {
        const auto currentTime = static_cast<float>(glfwGetTime());

        if (currentTime - s_LastSpaceTime < 0.3f) {
            s_IsFlying = !s_IsFlying;
            s_VerticalVelocity = 0.0f;
        }
        s_LastSpaceTime = currentTime;
    }
    s_SpaceWasPressed = currentSpace;

    if (s_IsFlying) {
        const float flySpeed = 10.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            cameraPos.y += flySpeed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            cameraPos.y -= flySpeed;
    }
    else {
        if (currentSpace && s_CoyoteTime > 0.0f && !s_IsFlying) {
            s_VerticalVelocity = s_JumpForce;
            s_CoyoteTime = 0.0f;
            // isGrounded = false;
        }
    }

    config::LevelData::get().setCameraPos(cameraPos);
}

void Player::processMovement(GLFWwindow* window, const float deltaTime) {

    if (config::LevelData::get().getCreativeModeBool()) {
        processCreativeMovement(window, deltaTime);
    } else {
        processSurvivalMovement(window, deltaTime);
    }

    static bool cPressed {false};
    const bool cIsDown {(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)};

    if (cIsDown && !cPressed) {
        auto& levelData = config::LevelData::get();
        levelData.setCreativeModeBool(!levelData.getCreativeModeBool());
    }

    cPressed = cIsDown;
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

    m_CameraFront = glm::normalize(front);
}

Camera::RaycastResult Camera::raycast(glm::vec3 start, glm::vec3 dir, float maxDist, worldgen::ChunkRendering& chunk) {
    RaycastResult result;
    if (glm::length2(dir) < 0.0001f) return result;

    dir = glm::normalize(dir);

    int x {static_cast<int>(std::floor(start.x))};
    int y {static_cast<int>(std::floor(start.y))};
    int z {static_cast<int>(std::floor(start.z))};

    int stepX {(dir.x > 0) ? 1 : -1};
    int stepY {(dir.y > 0) ? 1 : -1};
    int stepZ {(dir.z > 0) ? 1 : -1};

    float tMaxX {(stepX > 0) ? (std::floor(start.x) + 1.0f - start.x) / dir.x : (start.x - std::floor(start.x)) / -dir.x};
    float tMaxY {(stepY > 0) ? (std::floor(start.y) + 1.0f - start.y) / dir.y : (start.y - std::floor(start.y)) / -dir.y};
    float tMaxZ {(stepZ > 0) ? (std::floor(start.z) + 1.0f - start.z) / dir.z : (start.z - std::floor(start.z)) / -dir.z};

    float tDeltaX {(dir.x != 0) ? std::abs(1.0f / dir.x) : 1e30f};
    float tDeltaY {(dir.y != 0) ? std::abs(1.0f / dir.y) : 1e30f};
    float tDeltaZ {(dir.z != 0) ? std::abs(1.0f / dir.z) : 1e30f};

    glm::ivec3 lastNormal(0);
    float travelled {0.0f};

    while (travelled < maxDist) {
        // FIX: Read from the passed 'chunk' reference, NOT a local object!
        if (chunk.getBlockAt(x, y, z) != blockregistry::AIR) {
            result.hit = true;
            result.blockPos = glm::ivec3(x, y, z);
            result.placePos = result.blockPos - lastNormal;
            return result;
        }

        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) {
                travelled = tMaxX;
                tMaxX += tDeltaX;
                x += stepX;
                lastNormal = glm::ivec3(stepX, 0, 0);
            } else {
                travelled = tMaxZ;
                tMaxZ += tDeltaZ;
                z += stepZ;
                lastNormal = glm::ivec3(0, 0, stepZ);
            }
        } else {
            if (tMaxY < tMaxZ) {
                travelled = tMaxY;
                tMaxY += tDeltaY;
                y += stepY;
                lastNormal = glm::ivec3(0, stepY, 0);
            } else {
                travelled = tMaxZ;
                tMaxZ += tDeltaZ;
                z += stepZ;
                lastNormal = glm::ivec3(0, 0, stepZ);
            }
        }
    }
    return result;
}
}
