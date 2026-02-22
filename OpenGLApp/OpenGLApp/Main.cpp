#include <iostream>
#include <string>
#include <format>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "globals/scene_manager.h"
#include "globals/stats_manager.h"
#include "globals/score_manager.h"

#include "core/quad.h"

#include "characters/character.h"

#include "utils.h"

void framebufferSizeCallback(GLFWwindow * window, int width, int height);
void mouseCallback(GLFWwindow * window, int button, int action, int mods);

void processInput(GLFWwindow * window);

#define HDNT

#ifdef HD
    const unsigned int SCR_WIDTH = 1120;
    const unsigned int SCR_HEIGHT = 840;
#else
    const unsigned int SCR_WIDTH = 800;
    const unsigned int SCR_HEIGHT = 600;
#endif

int main() {
        // glfw: initialize and configure
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        // glfw window creation
        GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Wall-E: Clean Up Service", NULL, NULL);
        if (window == NULL) {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return -1;
        }

        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
        glfwSetMouseButtonCallback(window, mouseCallback);

        // glad: load all OpenGL function pointers
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return -1;
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Mesh
        Quad::instantiatePrimitive();

        // Scene initialization
        SceneManager::getInstance().changeScene(SceneManager::SceneID::MainMenuScene, window);

        ScoreManager::getInstance().load();

        // render loop
        while (!glfwWindowShouldClose(window)) {
            // window inputs
            processInput(window);

            // Scene update
            float delta = SceneManager::getInstance().currentScene->update();

            SceneManager::getInstance().ySortObjects();
            std::vector<std::shared_ptr<Character>> objectsInVec = SceneManager::getInstance().getObjects();

            // characters update
            for (std::shared_ptr<Character> object : SceneManager::getInstance()) {
                object->processInput(window);
                object->update(delta);
                object->collide(objectsInVec);
                object->renderSprite();
            }

            for (std::shared_ptr<Character> object : SceneManager::getInstance()) {
                object->resetCollisionState();
            }

            SceneManager::getInstance().currentScene->guiUpdate();

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        SceneManager::getInstance().currentScene->end();

        Quad::freePrimitive();

        ScoreManager::getInstance().set("current_score", std::to_string(StatsManager::getInstance().collectedBlocks));
        ScoreManager::getInstance().set("time", "00");
        ScoreManager::getInstance().save();

        // glfw: terminate, clearing all previously allocated GLFW resources.
        glfwTerminate();
        return 0;
    }

    // process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
    void processInput(GLFWwindow * window) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }

    // glfw: whenever the mouse moves, this callback is called
    void mouseCallback(GLFWwindow * window, int button, int action, int mods) {
        SceneManager::getInstance().currentScene->mouseCallback(window, button, action, mods);
    }

    // glfw: whenever the window size changed (by OS or user resize) this callback function executes
    void framebufferSizeCallback(GLFWwindow * window, int width, int height) {
        glViewport(0, 0, width, height);
        SceneManager::getInstance().currentScene->windowResizedCallback(window, width, height);
    }
