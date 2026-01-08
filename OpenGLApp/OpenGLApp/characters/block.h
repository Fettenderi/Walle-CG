#ifndef BLOCK_H
#define BLOCK_H

#include "../core/shader.h"
#include "../utils.h"

#include "character.h"

class Block : public Character {

    public:
        bool isPickable = true;

        Block(std::shared_ptr<Shader> spriteShader, glm::vec2 position, glm::vec2 scale)
            : Character(spriteShader, "assets/textures/block.png", CollisionShape(glm::vec2(0.0f, 0.0f), 0.1f, true), position, scale, 0.0f)
        {
            m_is_visible = false;
        }

        void setPosition(glm::vec2 position) {
            m_collider.isStatic = true;
            hasTarget = false;

            m_position = position;
        }

        void setRotation(float rotation) {
            m_rotation = rotation;
        }

        void getBlownTo(glm::vec2 pos, bool fromRight) {
            m_position = glm::vec2(fromRight ? 2.0f : -2.0f, pos.y);
            target = pos;
            hasTarget = true;
            m_collider.isStatic = false;
        }

        virtual void update(float deltaTime) {
            if (!hasTarget) return;

            float dist = glm::length(target - m_position);

            if (dist < 0.1) {
                hasTarget = false;
            }
            else {
                m_position = explerpVec2(m_position, target, deltaTime * 1.0f);
            }
        }

    private:
        glm::vec2 target;
        bool hasTarget = false;
};

#endif