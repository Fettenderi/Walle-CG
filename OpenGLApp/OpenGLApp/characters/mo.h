#ifndef MO_H
#define MO_H

#include "../shaders/shader_s.h"
#include "../utils.h"

#include "character.h"

class Mo : public Character {

    public:

        Mo(Shader * spriteShader, unsigned int* VAO, const char* texturePath,
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
        
        void setTarget(glm::vec2 pos) {
                target = pos;
                hasTarget = true;
        }  

        void update(float deltaTime) {

            if (hasTarget) {
                glm::vec2 pos = getPosition();
                m_direction = target - pos;

                float dist = glm::length(m_direction);

                if (dist < 0.01f) {
                    hasTarget = false; // arrivato
                }
                else {
                    m_direction = glm::normalize(m_direction);
                    m_position += m_direction * m_speed * deltaTime;
                }
            }
            //m_rotation = glm::degrees(atan2(m_direction.y, m_direction.x));
            }
  

    private:
        glm::vec2 m_direction;
        glm::vec2 m_velocity;
        glm::vec2 target;
        float m_speed;
        bool hasTarget = false;

};

#endif