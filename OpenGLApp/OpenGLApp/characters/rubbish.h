#ifndef RUBBISH_H
#define RUBBISH_H

#include "../core/shader.h"
#include "../utils.h"

#include "character.h"

class Rubbish : public Character {

    public:
        bool isPickable = false;

        Rubbish(std::shared_ptr<Shader> spriteShader, glm::vec2 position, glm::vec2 scale)
            : Character(spriteShader, "assets/textures/rubbish.png", CollisionShape(glm::vec2(0.0f, 0.0f), 0.3f), position, scale, 0.0f)
        {
            m_is_visible = false;
        }

        void setPosition(glm::vec2 position) {
            m_position = position;
        }
};

#endif