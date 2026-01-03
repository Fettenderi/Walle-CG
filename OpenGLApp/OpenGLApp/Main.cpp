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
#include "core/timer.h"
#include "core/pool.h"

#include "characters/character.h"
#include "characters/walle.h"
#include "characters/mo.h"
#include "characters/rubbish.h"
#include "characters/block.h"
#include "characters/camera.h"

#include "utils.h"

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
list<Shader*> shaders;
Shader* commonShader;
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
    Shader flatShader("core/shaders/flat_shader.vs", "core/shaders/flat_shader.fs");
    Shader lightedShader("core/shaders/lighted_shader.vs", "core/shaders/lighted_shader.fs");
    
    shaders.push_back(&flatShader);
    shaders.push_back(&lightedShader);

    commonShader = &flatShader;

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
    // soundManager -> play2D("assets/audio/getout.ogg", true);

    float delay = 0.0f;

    for (Shader *shader : shaders) {
       shader->use();
       shader->setInt("mainTexture", 0);
    }

    commonShader->use();
    commonShader->setInt("mainTexture", 0);

    lastElapsed = glfwGetTime();
    elapsed = glfwGetTime();

    camera.setSpeed(0.1f);
    camera.setMoving(false);


    //test pool
    ObjectPool<Rubbish> rubbishPool(3, &lightedShader, glm::vec2(0.0f, 0.0f), glm::vec2(0.5f, 0.4f));

    Rubbish* testRubbish = rubbishPool.getInstance();

    characters.push_back(testRubbish);
    int timeri = 0;

    Timer popTimer(4.0f, [&testRubbish, &timeri]() {
        if (timeri == 0) {
            characters.remove(testRubbish);
            timeri = 1;
        }
        else {
            characters.push_back(testRubbish);
            timeri = 0;
        
        }
        }, true);
    




    Block lightedBlock(&lightedShader, glm::vec2(0.1f, 0.0f), glm::vec2(10.0f, 10.0f));
    Walle walle(&lightedShader, glm::vec2(0.0f, 0.0f), glm::vec2(0.4f, 0.4f), 0.0f, 1.0f);
    Mo mo(&lightedShader, glm::vec2(0.0f, -0.6f), glm::vec2(0.28f, 0.4f), 0.0f, 1.0f);
    
    glfwSetWindowUserPointer(window, &mo);

    characters.push_back(&lightedBlock);
    characters.push_back(&mo);
    characters.push_back(&walle);

    glm::vec3 bgColor = glm::vec3(0.6f, 0.42f, 0.33f);
    glm::vec2 sunPosition = glm::vec2(1.0f, 0.0f);

    lightedShader.use();
    lightedShader.setVec3("ambientColor", hex_color("#a1d8e8"));
    lightedShader.setVec3("lightColor", hex_color("#ffffab"));

    glm::vec2 randomVec = glm::vec2(-0.5f, 0.5f);
    //test timer
    Timer testTimer(6.0f, [&mo, randomVec]() {
        mo.setTarget(randomVec);
        },
        false);

    

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // deltaTime calculation
        elapsed = glfwGetTime();
        deltaTime = elapsed - lastElapsed;
        lastElapsed = elapsed;
        //printf("current delta time: %f\n", deltaTime);

        testTimer.updateTimer(deltaTime);
        popTimer.updateTimer(deltaTime);

        // render
        glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window, &walle);
        
        camera.update((float)deltaTime);
        sunPosition = glm::vec2(cos(elapsed), sin(elapsed));

        for (Shader *shader : shaders) {
            shader->use();
            shader->setMat4("camera", camera.getViewMatrix());
        }

        lightedShader.use();
        lightedShader.setVec3("lightPosition", glm::vec3(sunPosition.x, 0.0f, sunPosition.y));
        lightedShader.setVec3("viewPosition", camera.getPosition());

        for (Character *character : characters) {
           character->processInput(window);
           character->update((float)deltaTime);
           character->renderSprite();
        }

        leText->RenderText(std::format("Collected trash: {}", walle.getCollected()), glm::vec2(0.0f, 0.0f), 1.0f, "#0a1518");

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
            Block blk(commonShader, wallePosition + glm::vec2(0.0f, 0.2f), glm::vec2(0.2f, 0.2f));

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