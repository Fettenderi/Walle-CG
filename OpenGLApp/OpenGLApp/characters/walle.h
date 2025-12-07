#ifndef WALLE_H
#define WALLE_H

#include "../shaders/shader_s.h"
#include "../utils.h"

#include "character.h"

class Walle : public Character {

    public:

        Walle(Shader * spriteShader, unsigned int* VAO, const char* texturePath,
            glm::vec2 position, glm::vec2 scale, const float rotation, const float speed)
            : Character(spriteShader, VAO, texturePath, position, scale, rotation), m_speed(speed)
        {
            m_direction = glm::vec2(1.0f, 0.0f);
        }

        void processInput(GLFWwindow* window) {
            m_velocity = glm::vec2(0.0f, 0.0f);

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                m_velocity += glm::vec2(0.0f, 1.0f);

            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                m_velocity -= glm::vec2(0.0f, 1.0f);

            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                m_velocity -= glm::vec2(1.0f, 0.0f);

            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                m_velocity += glm::vec2(1.0f, 0.0f);

            if (m_velocity == glm::vec2(0.0f, 0.0f)) return;

            m_velocity = glm::normalize(m_velocity);
            m_direction = m_velocity;
        }

        void update(float deltaTime) {
            m_rotation = glm::degrees(atan2(m_direction.y, m_direction.x));

            m_position += m_velocity * m_speed * deltaTime;
        }

    private:
        glm::vec2 m_direction;
        glm::vec2 m_velocity;
        float m_speed;

};

#endif