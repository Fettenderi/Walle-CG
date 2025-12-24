#include <iostream>
#include <list>
#include <random>
#include <string>
#include <format>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <irrKlang.h>

#include "core/text.h"
#include "core/shader.h"
#include "core/quad.h"

#include "characters/character.h"
#include "characters/walle.h"
#include "characters/mo.h"
#include "characters/rubbish.h"
#include "characters/block.h"
#include "characters/camera.h"


using namespace std;

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, int button, int action, int mods);

void processInput(GLFWwindow* window, Walle *walle);
    
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

double deltaTime;
double lastElapsed;
double elapsed;

Camera camera;
list<Character> istantiated;
list<Character*> characters;
Shader* shader;
Text* leText;

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
    Shader flatShader("core/flat_shader.vs", "core/flat_shader.fs");
    Shader lightedShader("core/lighted_shader.vs", "core/lighted_shader.fs");
    shader = &flatShader;

    // Text Provider
    Text text("assets/fonts/Antonio/static/Antonio-Bold.ttf");
    leText = &text;

    // Random Provider
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distf(-1.0f, 1.0f);

    // IrrKlang
    soundManager = irrklang::createIrrKlangDevice();

    if (!soundManager)
        return 0; // error starting up the engine
    
    // play some sound stream, looped
    soundManager -> play2D("assets/audio/getout.ogg", true);

    float delay = 0.0f;

    shader->use();
    shader->setInt("mainTexture", 0);

    lastElapsed = glfwGetTime();
    elapsed = glfwGetTime();

    camera.setShader(shader);
    camera.setSpeed(0.1f);
    camera.setMoving(false);

    Walle walle(shader, glm::vec2(0.0f, 0.0f), glm::vec2(0.4f, 0.4f), 0.0f, 1.0f);
    Mo mo(shader, glm::vec2(0.0f, -0.6f), glm::vec2(0.28f, 0.4f), 0.0f, 1.0f);
    
    glfwSetWindowUserPointer(window, &mo);

    characters.push_back(&mo);
    characters.push_back(&walle);


    // render loop
    while (!glfwWindowShouldClose(window)) {
        // deltaTime calculation
        elapsed = glfwGetTime();
        deltaTime = elapsed - lastElapsed;
        lastElapsed = elapsed;

        // render
        glClearColor(0.6f, 0.42f, 0.33f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window, &walle);

        camera.update((float)deltaTime);

        shader->use();
        for (Character *character : characters) {
            character->processInput(window);
            character->update((float)deltaTime);
            character->renderSprite();
        }

        if (delay < elapsed) {
            delay = (float)elapsed + 5.0f;
            Rubbish rubb(shader, glm::vec2(distf(gen), distf(gen)), glm::vec2(0.4f, 0.27f));

            istantiated.push_front(rubb);
            characters.push_front(&istantiated.front());
        }

        leText->RenderText(std::format("Collected trash: {}", walle.getCollected()), glm::vec2(0.0f, 0.0f), 1.0f, glm::vec3(0.5, 0.8f, 0.2f));

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Quad::freePrimitive();
    text.free();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window, Walle* walle) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        glm::vec2 wallePosition = walle->getPosition();
        bool collected = false;
        for (Character *rubbish : characters) {
            if ((rubbish != walle) && (glm::distance(rubbish->getPosition(), wallePosition) <= 0.2f)) {
                rubbish->free();
                collected = true;
                break;
            }
        }

        if (collected && walle->collect()) {
            Block blk(shader, wallePosition + glm::vec2(0.0f, 0.2f), glm::vec2(0.2f, 0.2f));

            istantiated.push_front(blk);
            characters.push_front(&istantiated.front());

        }
    }


    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        camera.setMoving(true);
    else 
        camera.setMoving(false);
}

// glfw: whenever the mouse moves, this callback is called
void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // Mo � nel window pointer
        Mo* mo = static_cast<Mo*>(glfwGetWindowUserPointer(window));
        double xpos, ypos;
        int width, height;

        glfwGetCursorPos(window, &xpos, &ypos);
        glfwGetWindowSize(window, &width, &height);

        //coordinate x schermata
        float scX = (float)xpos / (float)width * 2.0f - 1.0f;
        float scY = -(float)ypos / (float)height * 2.0f + 1.0f;

        //destinazione di Mo
        mo->setTarget(glm::vec2(scX, scY));
        //printf("MO target: (%f, %f)\n", scX, scY);
    }

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}