#ifndef WIND_H
#define WIND_H

#include "../core/shader.h"
#include "../core/timer.h"

#include "../globals/scene_manager.h"

#include "../utils.h"

#include "rubbish.h"
#include "block.h"
#include "character.h"

#include <memory>

class Wind : public Character {

    public:

        Wind(std::shared_ptr<ObjectPool<Rubbish>> rubbishPool, std::shared_ptr<ObjectPool<Block>> blockPool, std::shared_ptr<Shader> spriteShader, glm::vec2 position)
            : Character(spriteShader, "assets/textures/eve.png", CollisionShape(glm::vec2(0.0f, 0.0f), 0.1f, false), position, glm::vec2(0.1f), 0.0f), rubbishPool(rubbishPool), blockPool(blockPool)
        {
            m_direction = glm::vec2(1.0f, 0.0f);
            m_is_visible = false;
            strengthFactor = 1.0f / sqrt((float)BLOCK_COLUMNS * 5.0f);

            cooldownTimer = std::make_unique<Timer>(getNextRandomRange(15.0f, 33.0f), [this] {
                    tryGettingRubbish();
                } , false);
        }

        void update(float deltaTime) {
            cooldownTimer->updateTimer(deltaTime);
        }

        void updateStrength(float param) {
            strength = strengthFactor * sqrt(param);
        }

        float getStrength() {
            return strength;
        }


    private:
        glm::vec2 m_prev_position;

        glm::vec2 m_direction;
        glm::vec2 target;

        float m_speed;
        float strengthFactor;
        float strength = 0.0f;

        std::list<std::shared_ptr<Block>> blownBlocks;
        std::list<std::shared_ptr<Rubbish>> blownRubbish;

        std::unique_ptr<Timer> cooldownTimer;

        std::shared_ptr<ObjectPool<Rubbish>> rubbishPool;
        std::shared_ptr<ObjectPool<Block>> blockPool;

        void tryGettingRubbish() {
            setTarget(getRandomPosition(SceneManager::getInstance().camera->getPosition2D(), StatsManager::getInstance().maxBlockProgress) * 0.7f);
        }

        void setTarget(glm::vec2 pos) {
            //printf("(%f, %f)\n", pos.x, pos.y);

            int rubbishAmt = getNextRandomIntRange(6, 20);
            __nop();
            int blockAmt = getNextRandomIntRange(1, rubbishAmt - 1);
            rubbishAmt = rubbishAmt - blockAmt;

            rubbishAmt = clamp(0.0f, rubbishPool->getSize(), rubbishAmt);
            blockAmt = clamp(0.0f, blockPool->getSize(), blockAmt);

            if (rubbishAmt == 0 && blockAmt == 0) return;

            int i;

            for (i = 0; i < rubbishAmt; i++) {
               std::shared_ptr<Rubbish> rub = rubbishPool->getInstance();
               rub->show();
               rub->isPickable = false;
               rub->trashAmount = getNextRandomIntRange(1, (int)round(lerp((float)JUNK_TO_BLOCK / 5.0f, (float)JUNK_TO_BLOCK / 2.0f, strength)));
               rub->getBlownTo(clampInCamera(pos + getRandomVector() * 0.3f, SceneManager::getInstance().camera->getPosition2D(), StatsManager::getInstance().maxBlockProgress), pos.x < 0.0f);
               blownRubbish.push_back(rub);
               SceneManager::getInstance().addObject(rub);
            }

            for (i = 0; i < blockAmt; i++) {
               std::shared_ptr<Block> blk = blockPool->getInstance();
               blk->show();
               blk->isPickable = true;
               blk->getBlownTo(clampInCamera(pos + getRandomVector() * 0.3f, SceneManager::getInstance().camera->getPosition2D(), StatsManager::getInstance().maxBlockProgress), pos.x < 0.0f);
               blownBlocks.push_back(blk);
               SceneManager::getInstance().addObject(blk);
            }

            // cooldownTimer->changeDuration(1.0f);
            float min = fmax(lerp(33.0f, 5.0f, strength), 1.0f);
            float max = fmax(lerp(50.0f, 30.0f, strength), 1.0f);
            //printf("%f, %f\n", min, max);
            cooldownTimer->changeDuration(getNextRandomRange(min, max));
            cooldownTimer->resume();
            cooldownTimer->reset();
        }
};

#endif