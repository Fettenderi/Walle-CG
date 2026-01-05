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

#include "utils.h"

using namespace std;

void framebufferSizeCallback(GLFWwindow * window, int width, int height);
void mouseCallback(GLFWwindow * window, int button, int action, int mods);

void processInput(GLFWwindow * window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

double deltaTime;
double lastElapsed;
double elapsed;

std::shared_ptr<Camera> camera;
std::shared_ptr<Mo> mo;
list<std::shared_ptr<Shader>> shaders;
std::unique_ptr<Text> guiText;

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

        // Shaders
        //std::shared_ptr<Shader> flatShader = std::make_shared<Shader>("core/shaders/flat_shader.vs", "core/shaders/flat_shader.fs");
        std::shared_ptr<Shader> lightedShader = std::make_shared<Shader>("core/shaders/lighted_shader.vs", "core/shaders/lighted_shader.fs");

        //shaders.push_back(flatShader);
        shaders.push_back(lightedShader);

        // Text Provider
        guiText = make_unique<Text>("assets/fonts/Antonio/static/Antonio-Bold.ttf");

        // IrrKlang
        soundManager = irrklang::createIrrKlangDevice();

        if (!soundManager)
            return 0; // error starting up the engine

        lastElapsed = glfwGetTime();
        elapsed = glfwGetTime();

        // play some sound stream, looped
        // soundManager -> play2D("assets/audio/getout.ogg", true);

        //for (std::shared_ptr<Shader> shader : shaders) {
        //    shader->use();
        //    shader->setInt("mainTexture", 0);
        //}

        // camera
        SceneManager::getInstance().camera = make_shared<Camera>();
        camera = SceneManager::getInstance().camera;

        camera->setSpeed(0.4f);
        camera->setMoving(false);

        // pools initialization
        SceneManager::getInstance().rubbishPool = make_unique<ObjectPool<Rubbish>>(5, lightedShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.4f, 0.32f));
        SceneManager::getInstance().blockPool = make_unique<ObjectPool<Block>>(23, lightedShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.25f, 0.25f));

        // background
        std::shared_ptr<Block> lightedBlock = make_shared<Block>(lightedShader, glm::vec2(0.0f, 0.0f), glm::vec2(10.0f, 10.0f));
        lightedBlock->isPickable = false;
        lightedBlock->show();

        // characters
        std::shared_ptr<Walle> walle = make_shared<Walle>(lightedShader, glm::vec2(0.0f, 0.0f), glm::vec2(0.4f, 0.4f), 0.0f, 1.0f);
        std::shared_ptr<Eve> eve = make_shared<Eve>(lightedShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.28f, 0.4f), 1.0f);
        mo = make_shared<Mo>(lightedShader, glm::vec2(0.0f, -0.6f), glm::vec2(0.28f, 0.4f), 0.0f, 1.0f);

        SceneManager::getInstance().addObject(lightedBlock);
        SceneManager::getInstance().addObject(mo);
        SceneManager::getInstance().addObject(walle);
        SceneManager::getInstance().addObject(eve);

        // lights setup
        std::shared_ptr<Light> sun = make_shared<Light>(glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, "#ffffab");
        SceneManager::getInstance().sun = sun;

        lightedShader->use();
        lightedShader->setInt("mainTexture", 0);

        glm::vec3 bgColor = glm::vec3(0.6f, 0.42f, 0.33f);

        lightedShader->setVec3("ambientColor", hex_color("#a1d8e8"));
        lightedShader->setVec3("sunColor", sun->getColor());
        lightedShader->setFloat("sunStrength", sun->strength);

        // render loop
        while (!glfwWindowShouldClose(window)) {
            // deltaTime calculation
            elapsed = glfwGetTime();
            deltaTime = elapsed - lastElapsed;
            lastElapsed = elapsed;

            // render
            glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // window inputs
            processInput(window);

            // global update
            camera->update((float)deltaTime);
            sun->position = glm::vec3(sin(elapsed * 0.1f), 0.0f, cos(elapsed * 0.1f));

            walle->setFlashlight(cos(elapsed * 0.1f) <= 0.0f);

            // shader update
            //for (std::shared_ptr<Shader> shader : shaders) {
            //    shader->use();
            //    shader->setMat4("camera", camera->getViewMatrix());
            //}

            lightedShader->use();
            lightedShader->setMat4("camera", camera->getViewMatrix());
            lightedShader->setVec3("sunPosition", sun->position);
            lightedShader->setVec3("viewPosition", camera->getPosition());

            // characters update
            for (std::shared_ptr<Character> object : SceneManager::getInstance()) {
                object->processInput(window);
                object->update((float)deltaTime);
                object->renderSprite();
            }

            // gui update
            guiText->RenderText(std::format("Collected trash: {}", walle->getCollected()), glm::vec2(0.0f, 0.0f), 1.0f, "#0a1518");

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

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
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            double xpos, ypos;
            int width, height;

            glfwGetCursorPos(window, &xpos, &ypos);
            glfwGetWindowSize(window, &width, &height);

            //coordinate x schermata
            float scX = (float)xpos / (float)width * 2.0f - 1.0f;
            float scY = -(float)ypos / (float)height * 2.0f + 1.0f;

            //destinazione di Mo
            mo->setTarget(glm::vec2(scX, scY) - camera->getPosition2D());
            //printf("MO target: (%f, %f)\n", scX, scY);
        }

    }

    // glfw: whenever the window size changed (by OS or user resize) this callback function executes
    void framebufferSizeCallback(GLFWwindow * window, int width, int height) {
        glViewport(0, 0, width, height);
    }
