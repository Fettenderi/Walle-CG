#ifndef RUBBISH_H
#define RUBBISH_H

#include "../core/shader.h"
#include "../utils.h"

#include "character.h"

class Rubbish : public Character {

    public:
        bool isPickable = false;

        Rubbish(std::shared_ptr<Shader> spriteShader, glm::vec2 position, glm::vec2 scale)
            : Character(spriteShader, "assets/textures/rubbish.png", CollisionShape(glm::vec2(0.0f, 0.0f), 0.1f, true), position, scale, 0.0f)
        {
            m_is_visible = false;
        }

        void setPosition(glm::vec2 position) {
            m_position = position;
        }

        void getBlownTo(glm::vec2 pos, bool fromRight) {
            m_position = glm::vec2(fromRight ? 2.0f : -2.0f, pos.y);
        }

        virtual void update(float deltaTime) {
            if (!hasTarget) return;

            m_position = explerpVec2(m_position, target, deltaTime * 1.0f);
        }


    private:
        glm::vec2 target;
        bool hasTarget = false;
};

#endif