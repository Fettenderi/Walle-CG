#pragma once

#include "../core/shader.h"
#include "../core/timer.h"

#include "../globals/scene_manager.h"
#include "../globals/stats_manager.h"

#include "../utils.h"

#include <memory>
#include <list>
#include <format>
#include <iostream>
#include <string>

class RubbishSpawner : public Character {
    public:
        RubbishSpawner(std::shared_ptr<Shader> spriteShader)
            : Character(spriteShader, "assets/textures/eve.png", CollisionShape(glm::vec2(0.0f, 0.0f), 0.1f, false), glm::vec2(2.0f, 2.0f), glm::vec2(0.01f, 0.01f), 0.0f) {

            m_is_visible = false;
            m_collider.isActive = false;
        }

        virtual ~RubbishSpawner() {
            rubbishPool.reset();
        }

    private:
        std::shared_ptr<ObjectPool<Character>> rubbishPool;

        std::shared_ptr<Character> spawnRubbish(glm::vec2 position, bool isPickable);
};