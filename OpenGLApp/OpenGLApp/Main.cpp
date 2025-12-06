#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_s.h"
#include "camera.h"
#include "character.h"

#include <iostream>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);

void processInput(GLFWwindow* window, Character &character);
void instantiatePrimitive(unsigned int* VAO, unsigned int* VBO, unsigned int* EBO, float* vertices, size_t verticesSize, unsigned int* indices, size_t indicesSize);
void loadSprite(Shader shader, unsigned int VAO, unsigned int texture, glm::vec2 position, glm::vec2 scale, float rotation);
void loadTexture(unsigned int* texture, const char* textureSource, GLint colorEncoding);
    
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);

bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

double deltaTime;
double lastElapsed;
double elapsed;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // glfw window creation
    // --------------------
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
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("shader.vs", "shader.fs"); // you can name your shader files however you like

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
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

    unsigned int VBO, VAO, EBO;
    instantiatePrimitive(&VAO, &VBO, &EBO, vertices, sizeof(vertices), indices, sizeof(indices));

    unsigned int textures[3];
    loadTexture(&(textures[0]), "container.jpg", GL_RGB);
    loadTexture(&(textures[1]), "container-demo.jpg", GL_RGB);
    loadTexture(&(textures[2]), "awesomeface.png", GL_RGBA);

    ourShader.use();
    ourShader.setInt("mainTexture", 0);

    lastElapsed = glfwGetTime();
    elapsed = glfwGetTime();

    Character player(glm::vec2(0.0f, 0.0f), 0.0f, "awesomeface.png");

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // deltaTime calculation
        // -----------------
        elapsed = glfwGetTime();
        deltaTime = elapsed - lastElapsed;
        lastElapsed = elapsed;

        // input
        // -----
        processInput(window, player);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        loadSprite(ourShader, VAO, player.getTextureID(), player.getPosition(), glm::vec2(1.0f, 1.0f), player.getRotationDeg());
        loadSprite(ourShader, VAO, textures[0], glm::vec2(0.2f, 0.5f), glm::vec2(1.0f, 1.0f), 0.0f);
        loadSprite(ourShader, VAO, textures[1], glm::vec2(-0.2f, 0.5f), glm::vec2(0.5f, 0.5f), 90.0f);
        loadSprite(ourShader, VAO, textures[1], glm::vec2(-0.2f, -0.5f), glm::vec2(0.5f, 0.5f), elapsed * 20.0f);
        loadSprite(ourShader, VAO, textures[2], glm::vec2(sin(elapsed), -0.5f), glm::vec2(0.5f, 0.5f), elapsed * 10.0f);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, Character &character)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    
    const float speed = 0.05f; // velocit? di movimento

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        character.move(0.0f, speed);

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        character.move(0.0f, -speed);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        character.move(-speed, 0.0f);

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        character.move(speed, 0.0f);
    
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
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
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
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
