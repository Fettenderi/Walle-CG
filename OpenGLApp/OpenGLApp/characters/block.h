#ifndef BLOCK_H
#define BLOCK_H

#include "../core/shader.h"
#include "../utils.h"
#include "i_pickable.h"

#include "character.h"

class Block : public Character, public IPickable {

    public:

        Block(Shader * spriteShader, glm::vec2 position, glm::vec2 scale)
            : Character(spriteShader, "assets/textures/block.png", position, scale, 0.0f)
        {
        }

        virtual bool canBePicked(Character* picker) {
            return dynamic_cast<Mo*>(picker) != nullptr;
        }

        void free() {}
};

#endif