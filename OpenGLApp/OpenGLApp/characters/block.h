#ifndef BLOCK_H
#define BLOCK_H

#include "../core/shader.h"
#include "../utils.h"

#include "character.h"

class Block : public Character {

    public:
        bool isPickable = true;

        Block(std::shared_ptr<Shader> spriteShader, glm::vec2 position, glm::vec2 scale)
            : Character(spriteShader, "assets/textures/block.png", CollisionShape(glm::vec2(0.0f, 0.0f), 0.4f), position, scale, 0.0f)
        {
            m_is_visible = false;
        }

        void setPosition(glm::vec2 position) {
            m_position = position;
        }

        void setRotation(float rotation) {
            m_rotation = rotation;
        }
};

#endif