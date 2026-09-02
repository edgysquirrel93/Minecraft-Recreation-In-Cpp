#include "Input.h"

#include <cmath>
#include <glm/gtx/norm.hpp>

#include "Engine/ui/UIManager.h"
#include "Engine/config/SettingsManager.h"
#include "Engine/worldgen/WorldGen.h"

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

bool Input::grounded(const glm::vec3& pos)
{
    constexpr float EYE_HEIGHT {1.62f};

    const int blockX {static_cast<int>(std::floor(pos.x))};
    const int blockY {static_cast<int>(std::floor(pos.y - EYE_HEIGHT))};
    const int blockZ {static_cast<int>(std::floor(pos.z))};

    const auto* world {config::LevelData::get().getWorld()};
    if (!world) return false;

    return world->getBlockAt(blockX, blockY, blockZ) != blockregistry::get(blockregistry::ID_AIR);
}

// bool Input::checkCollision(const glm::vec3& pos)
// {
//     constexpr float EYE_HEIGHT = 1.62f;
//
//     const int blockX {static_cast<int>(std::floor(pos.x))};
//     const int bottomBlockY {static_cast<int>(std::floor(pos.y - EYE_HEIGHT + 0.5f))};
//     const int topBlockY {static_cast<int>(std::floor(pos.y - 0.12f))};
//     const int headBlockY {static_cast<int>(std::floor(pos.y + 0.38f))};
//     const int blockZ {static_cast<int>(std::floor(pos.z))};
//
//     const auto* chunk = config::LevelData::get().getActiveChunk();
//     if (!chunk) return false;
//
//     if ()
//
//     if (chunk->getBlockAt)
//     // check all 4 bottom corners and 4 top corners
//     for (float dx : {-halfWidth, halfWidth}) {
//         for (float dz : {-halfWidth, halfWidth}) {
//             // feet level
//             if (!isAir((int)floor(x + dx), (int)floor(y - height), (int)floor(z + dz))) return true;
//             // mid-level
//             if (!isAir((int)floor(x + dx), (int)floor(y - height * 0.5f), (int)floor(z + dz))) return true;
//             // head level
//             if (!isAir((int)floor(x + dx), (int)floor(y), (int)floor(z + dz))) return true;
//         }
//     }
//     return false;
// }

void Player::processGravity(const float deltaTime) {
    glm::vec3 cameraPos {config::LevelData::get().getCameraPos()};
    s_VerticalVelocity += s_Gravity * deltaTime;

    glm::vec3 targetPos = cameraPos;
    targetPos.y += s_VerticalVelocity * deltaTime;

    if (s_VerticalVelocity <= 0.0f && Input::grounded(targetPos)) {
        constexpr float EYE_HEIGHT = 1.62f;
        s_IsGrounded = true;
        s_CoyoteTime = s_CoyoteDuration;

        const int blockY = static_cast<int>(std::floor(targetPos.y - EYE_HEIGHT));
        cameraPos.y = static_cast<float>(blockY + 1) + EYE_HEIGHT;

        s_VerticalVelocity = 0.0f;
    }
    else {
        cameraPos.y = targetPos.y;
        s_IsGrounded = false;
        s_CoyoteTime -= deltaTime;
    }

    config::LevelData::get().setCameraPos(cameraPos);
}

void Player::processSurvivalMovement(GLFWwindow* window, const float deltaTime) {

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
            speed *= 1.3f;
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

    if (!s_IsFlying) {
        processGravity(deltaTime);

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && s_CoyoteTime > 0.0f) {
            s_VerticalVelocity = s_JumpForce;
            s_CoyoteTime = 0.0f;
            s_IsGrounded = false;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) s_BuildingBlock = blockregistry::ID_GRASS;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) s_BuildingBlock = blockregistry::ID_DIRT;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) s_BuildingBlock = blockregistry::ID_STONE;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) s_BuildingBlock = blockregistry::ID_BEDROCK;
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) s_BuildingBlock = blockregistry::ID_GLASS;

    auto [pressed, blockPos, placePos] {Camera::raycast(config::LevelData::get().getCameraPos(),
Camera::getCameraFront(), 5.0f, *config::LevelData::get().getWorld())};

    bool currentLeft = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (currentLeft && !s_LeftMousePressed) {
        if (pressed) {
            config::LevelData::get().getWorld()->setBlockAt(blockPos.x, blockPos.y, blockPos.z, blockregistry::ID_AIR);
        }
    }
    s_LeftMousePressed = currentLeft;

    bool currentRight {glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS};
    if (currentRight && !s_RightMousePressed) {
        if (pressed) {
                config::LevelData::get().getWorld()->setBlockAt(placePos.x, placePos.y, placePos.z, s_BuildingBlock);
            }
        }
    s_RightMousePressed = currentRight;
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
            s_IsGrounded = false;
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

Camera::RaycastResult Camera::raycast(glm::vec3 start, glm::vec3 dir, float maxDist, worldgen::World& world) {
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
        if (world.getBlockAt(x, y, z) != blockregistry::get(blockregistry::ID_AIR)) {
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
