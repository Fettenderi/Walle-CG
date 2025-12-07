#ifndef CHARACTER_H
#define CHARACTER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders/shader_s.h"

class Character {
protected:
    glm::vec2 m_position;
    glm::vec2 m_scale;
    float m_rotation;
    char* m_texture_path;
    unsigned int m_textureID;
    unsigned int* m_VAO;
    Shader* m_shader;

    bool m_freed = false;

public:

    Character(Shader* spriteShader, unsigned int* VAO, const char* texturePath, glm::vec2 position, glm::vec2 scale, const float rotation)
        : m_position(position), m_scale(scale), m_rotation(rotation), m_VAO(VAO), m_shader(spriteShader) {
        loadTexture(&m_textureID, texturePath, GL_RGBA);
    }

    void renderSprite() {
        if (m_freed) return;

        loadSprite(*m_shader, *m_VAO, m_textureID, m_position, m_scale, m_rotation);
    }

    virtual void processInput(GLFWwindow* window) {}

    virtual void update(float deltaTime) {}

    glm::vec2 getPosition() const { return m_position; }

    void free() {
        m_freed = true;
    }

private:
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
};

#endif