#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Character
{
public:
    unsigned int ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Character(const glm::vec2 position, const float deg, const char* texture_path)
    {
        m_position = position;
        m_deg = deg;
        loadTexture(&m_textureID, texture_path, GL_RGBA);

    }


    

    // --- GETTERS ---
    glm::vec2 getPosition() const { return m_position; }
    float getRotationDeg() const { return m_deg; }
    unsigned int getTextureID() const { return m_textureID; }

    // --- SETTERS ---
    void setPosition(const glm::vec2& pos) { m_position = pos; }
    void setRotationDeg(float deg) { m_deg = deg; }


    // ---------------
    void move(float dx, float dy) {
        m_position.x += dx;
        m_position.y += dy;
    }

private:
    // --- VARIABILI MEMBRO ---
    glm::vec2 m_position;
    float m_deg;
    char* m_texture_path;
    unsigned int m_textureID;



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
};

#endif