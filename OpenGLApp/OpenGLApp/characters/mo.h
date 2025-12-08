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

        void setTarget(glm::vec2 pos) {
            target = pos;
            hasTarget = true;
        }  

        void update(float deltaTime) {

            if (!hasTarget) return;

            glm::vec2 pos = getPosition();
            m_direction = target - pos;

            m_rotation = glm::degrees(atan2(-m_direction.y, m_direction.x));
            m_scale.y = glm::abs(m_scale.y) * sign(m_direction.x);

            float dist = glm::length(m_direction);

            if (dist < 0.01f) {
                hasTarget = false; // arrivato
            }
            else {
                m_direction = glm::normalize(m_direction);
                m_position += m_direction * m_speed * deltaTime;

            }
        }

        void free() {}

  

    private:
        glm::vec2 m_direction;
        glm::vec2 m_velocity;
        glm::vec2 target;
        float m_speed;
        bool hasTarget = false;

};

#endif