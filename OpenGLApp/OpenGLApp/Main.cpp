#include <iostream>
#include <string>
#include <format>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <irrKlang.h>

#include "globals/scene_manager.h"
#include "globals/stats_manager.h"

#include "core/text.h"
#include "core/shader.h"
#include "core/quad.h"
#include "core/timer.h"
#include "core/pool.h"
#include "core/light.h"
#include "core/camera.h"

#include "characters/character.h"
#include "characters/walle.h"
#include "characters/mo.h"
#include "characters/eve.h"
#include "characters/rubbish.h"
#include "characters/block.h"
#include "characters/image.h"

#include "scenes/game_scene.h"
#include "scenes/test_scene.h"
#include "scenes/main_menu_scene.h"

#include "utils.h"

void framebufferSizeCallback(GLFWwindow * window, int width, int height);
void mouseCallback(GLFWwindow * window, int button, int action, int mods);

void processInput(GLFWwindow * window);

#define HD

#ifdef HD
    const unsigned int SCR_WIDTH = 1120;
    const unsigned int SCR_HEIGHT = 840;
#else
    const unsigned int SCR_WIDTH = 800;
    const unsigned int SCR_HEIGHT = 600;
#endif

std::shared_ptr<Scene> currentScene;

irrklang::ISoundEngine* soundManager;

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
        GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Walle-Demo", NULL, NULL);
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

        // glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Mesh
        Quad::instantiatePrimitive();

        // IrrKlang
        soundManager = irrklang::createIrrKlangDevice();

        if (!soundManager)
            return 0; // error starting up the engine

        // play some sound stream, looped
        // soundManager -> play2D("assets/audio/getout.ogg", true);

        // Scene initialization
        currentScene = std::make_shared<MainMenuScene>(window);
        SceneManager::getInstance().currentScene = currentScene;

        SceneManager::getInstance().currentScene->init();

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

            currentScene->guiUpdate();

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        SceneManager::getInstance().currentScene->end();

        Quad::freePrimitive();

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
    }
