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
        float strength = 0.0f;

        Wind(std::shared_ptr<ObjectPool<Rubbish>> rubbishPool, std::shared_ptr<ObjectPool<Block>> blockPool, std::shared_ptr<Shader> spriteShader, glm::vec2 position)
            : Character(spriteShader, "assets/textures/eve.png", CollisionShape(glm::vec2(0.0f, 0.0f), 0.1f, false), position, glm::vec2(0.1f), 0.0f), rubbishPool(rubbishPool), blockPool(blockPool)
        {
            m_direction = glm::vec2(1.0f, 0.0f);
            m_is_visible = false;


            cooldownTimer = std::make_unique<Timer>(getNextRandomRange(15.0f, 33.0f), [this] {
                    tryGettingRubbish();
                } , false);
        }


        void update(float deltaTime) {
            cooldownTimer->updateTimer(deltaTime);
        }

    private:
        glm::vec2 m_prev_position;

        glm::vec2 m_direction;
        glm::vec2 target;

        float m_speed;

        std::list<std::shared_ptr<Block>> blownBlocks;
        std::list<std::shared_ptr<Rubbish>> blownRubbish;

        std::unique_ptr<Timer> cooldownTimer;

        std::shared_ptr<ObjectPool<Rubbish>> rubbishPool;
        std::shared_ptr<ObjectPool<Block>> blockPool;

        void tryGettingRubbish() {
            setTarget(getRandomPosition(SceneManager::getInstance().camera->getPosition2D(), StatsManager::getInstance().maxBlockProgress) * 0.7f);
        }

        void setTarget(glm::vec2 pos) {
            int rubbishAmt = (int)round(getNextRandomRange(5.0f, 12.0f));
            int blockAmt = (int)round(getNextRandomRange(1.0f, (float)rubbishAmt) - 1.0f);
            rubbishAmt = rubbishAmt - blockAmt;

            rubbishAmt = (int)fmin(rubbishAmt, rubbishPool->getSize());
            blockAmt = (int)fmin(blockAmt, blockPool->getSize());

            if (rubbishAmt == 0 && blockAmt == 0) return;

            int i;

            for (i = 0; i < rubbishAmt; i++) {
                std::shared_ptr<Rubbish> rub = rubbishPool->getInstance();
                rub->show();
                rub->isPickable = false;
                rub->getBlownTo(clampInCamera(pos + getRandomVector() * 0.3f, SceneManager::getInstance().camera->getPosition2D(), StatsManager::getInstance().maxBlockProgress), pos.x < 0.0f);
                blownRubbish.push_back(rub);
                SceneManager::getInstance().addObject(rub);
            }

            for (i = 0; i < blockAmt; i++) {
                std::shared_ptr<Block> blk = blockPool->getInstance();
                blk->show();
                blk->isPickable = false;
                blk->getBlownTo(clampInCamera(pos + getRandomVector() * 0.3f, SceneManager::getInstance().camera->getPosition2D(), StatsManager::getInstance().maxBlockProgress), pos.x < 0.0f);
                blownBlocks.push_back(blk);
                SceneManager::getInstance().addObject(blk);
            }
        }

};

#endif