#ifndef RUBBISH_H
#define RUBBISH_H

#include "../core/shader.h"
#include "../utils.h"
#include "i_pickable.h"

#include "character.h"

class Rubbish : public Character, public IPickable {

    public:

        Rubbish(Shader * spriteShader, glm::vec2 position, glm::vec2 scale)
            : Character(spriteShader, "assets/textures/rubbish.png", position, scale, 0.0f)
        {
        }

        virtual bool canBePicked(Character* picker) {
            return dynamic_cast<Walle*>(picker) != nullptr;
        }

        void free() {}

};

#endif