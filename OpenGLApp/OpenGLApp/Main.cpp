#include <iostream>
#include <list>
#include <random>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"
#include "shaders/shader_s.h"
#include "characters/character.h"
#include "characters/walle.h"
#include "characters/mo.h"

using namespace std;

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void processInput(GLFWwindow* window, Walle *walle);
void instantiatePrimitive(unsigned int* VAO, unsigned int* VBO, unsigned int* EBO, float* vertices, size_t verticesSize, unsigned int* indices, size_t indicesSize);
void loadSprite(Shader shader, unsigned int VAO, unsigned int texture, glm::vec2 position, glm::vec2 scale, float rotation);
void loadTexture(unsigned int* texture, const char* textureSource, GLint colorEncoding);
    
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

double deltaTime;
double lastElapsed;
double elapsed;

class Camera {
    private:
        glm::vec2 position;
        float speed;
        Shader* shader;
        bool canMove;

    public:
        Camera() {
            position = glm::vec2(0.0f, 0.0f);
            speed = 0.0f;
            canMove = false;
        }

        void update(float deltaTime) {
            if (!canMove) return;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(position, 0.0f));

            position += glm::vec2(0.0f, -deltaTime * speed);

            shader->setMat4("camera", model);
        }

        bool isMoving() {
            return canMove;
        }

        void setMoving(bool value) {
            canMove = value;
        }

        void setShader(Shader *shader) {
            this->shader = shader;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(position, 0.0f));

            this->shader->setMat4("camera", model);

        }

        void setSpeed(float speed) {
            this->speed = speed;
        }
};

Camera camera;
list<Character> istantiated;
list<Character*> characters;
Shader * shader;
unsigned int VAO;

int main()
{
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
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    Shader ourShader("shaders/shader.vs", "shaders/shader.fs"); // you can name your shader files however you like

    shader = &ourShader;

    // set up vertex data (and buffer(s)) and configure vertex attributes
    float vertices[] = {
           // position        // texture
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // top left 
    };

    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distf(-1.0f, 1.0f);

    float delay = 0.0f;

    unsigned int VBO, EBO;
    instantiatePrimitive(&VAO, &VBO, &EBO, vertices, sizeof(vertices), indices, sizeof(indices));

    ourShader.use();
    ourShader.setInt("mainTexture", 0);

    lastElapsed = glfwGetTime();
    elapsed = glfwGetTime();

    camera.setShader(&ourShader);
    camera.setSpeed(0.1f);
    camera.setMoving(false);

    Walle walle(&ourShader, &VAO, "assets/textures/walle.png", glm::vec2(0.0f, 0.0f), glm::vec2(0.4f, 0.4f), 0.0f, 1.0f);
    Mo mo(&ourShader, &VAO, "assets/textures/mo.png", glm::vec2(0.0f, -0.6f), glm::vec2(0.28f, 0.4f), 0.0f, 1.0f);

    glfwSetWindowUserPointer(window, &mo);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    
    characters.push_back(&mo);
    characters.push_back(&walle);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // deltaTime calculation
        elapsed = glfwGetTime();
        deltaTime = elapsed - lastElapsed;
        lastElapsed = elapsed;

        // render
        glClearColor(0.6f, 0.42f, 0.33f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        processInput(window, &walle);

        camera.update((float)deltaTime);

        for (Character *character : characters) {
            character->processInput(window);

            character->update((float)deltaTime);

            character->renderSprite();
        }


        if (delay < elapsed) {
            delay = elapsed + 5.0f;
            Character ch(&ourShader, &VAO, "assets/textures/rubbish.png", glm::vec2(distf(gen), distf(gen)), glm::vec2(0.4f, 0.27f), 0.0f);

            istantiated.push_front(ch);
            characters.push_front(&istantiated.front());
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window, Walle* walle)
{
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
            Character ch(shader, &VAO, "assets/textures/block.png", wallePosition + glm::vec2(0.0f, 0.2f), glm::vec2(0.2f, 0.2f), 0.0f);

            istantiated.push_front(ch);
            characters.push_front(&istantiated.front());

        }
    }


    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        camera.setMoving(true);
    else 
        camera.setMoving(false);
}

// glfw: whenever the mouse moves, this callback is called
void mouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
void instantiatePrimitive(unsigned int* VAO, unsigned int* VBO, unsigned int* EBO, float* vertices, size_t verticesSize, unsigned int* indices, size_t indicesSize) {
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);
    glGenBuffers(1, EBO);

    glBindVertexArray(*VAO);

    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void loadSprite(Shader shader, unsigned int VAO, unsigned int texture, glm::vec2 position, glm::vec2 scale, float rotation) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(VAO);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, -1.0f));
    model = glm::scale(model, glm::vec3(scale, 1.0f));

    shader.setMat4("model", model);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void loadTexture(unsigned int* texture, const char* textureSource, GLint colorEncoding) {
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    //stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis. 
    unsigned char* data = stbi_load(textureSource, &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, colorEncoding, width, height, 0, colorEncoding, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Mo è nel window pointer
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

