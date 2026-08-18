#ifndef MINECRAFT_RECREATION_RECREATION_INPUT_H
#define MINECRAFT_RECREATION_RECREATION_INPUT_H
#include "GLFW/glfw3.h"

namespace engine::input
{
class Input
{
public:
    // void updatePlayer(float deltaTime);
    static void processInput(GLFWwindow* window);
};
}

#endif //MINECRAFT_RECREATION_RECREATION_INPUT_H
