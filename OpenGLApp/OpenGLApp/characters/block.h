#ifndef BLOCK_H
#define BLOCK_H

#include "../core/timer.h"
#include "../core/shader.h"
#include "../core/pool.h"

#include "../utils.h"

#include "../globals/scene_manager.h"
#include "../globals/stats_manager.h"
#include "../globals/file_manager.h"

#include "character.h"
#include "rubbish.h"

class Block : public Character {

    public:
        bool isPickable = true;

        Block(std::shared_ptr<Shader> spriteShader, glm::vec2 position, glm::vec2 scale)
            : Character(spriteShader, "assets/textures/block.png", CollisionShape(glm::vec2(0.0f, 0.0f), 0.1f, true), position, scale, 0.0f)
        {
            m_is_visible = false;

            splitStrength = to_float(FileManager::getInstance().get(FileManager::CONFIG, "split_block_strength"));
            if (splitStrength == 0.0f) {
                splitStrength = 0.3f;
                FileManager::getInstance().set(FileManager::CONFIG, "split_block_strength", std::to_string(splitStrength));
            }

            float splitTime = to_float(FileManager::getInstance().get(FileManager::CONFIG, "split_block_time"));
            if (splitTime == 0.0f) {
                splitTime = 8.0f;
                FileManager::getInstance().set(FileManager::CONFIG, "split_block_time", std::to_string(splitTime));
            }

            splittingCountdown = std::make_unique<Timer>(abs(getNormalRandomClamped(splitTime, 1.0f)), [this] {
                split();
                }, false);

            splittingCountdown->pause();
        }

        ~Block() {
            rubbishPool.reset();
            blockPool.reset();

            splittingCountdown.release();
        }

        void clampPosition(glm::vec2 min, glm::vec2 max) {
            if (!m_is_visible) return;
            if (!isPickable) return;
            //if (isFixed) return;

            m_position = clamp(min, max, m_position);
        }

        void setPosition(glm::vec2 position) {
            m_collider.isStatic = true;
            hasTarget = false;

            m_position = position;
        }

        void setRotation(float rotation) {
            m_rotation = rotation;
        }

        void setRubbishPool(std::shared_ptr<ObjectPool<Rubbish>> pool) {
            rubbishPool = pool;
        }

        void setBlockPool(std::shared_ptr<ObjectPool<Block>> pool) {
            blockPool = pool;
        }

        void setSplittable(bool value) {
            if (value) {
                isFixed = false;
                splittingCountdown->reset();
                splittingCountdown->resume();
            }
            else {
                isFixed = true;
                splittingCountdown->pause();
            }
        }

        void getBlownTo(glm::vec2 pos, bool fromRight) {
            m_position = glm::vec2(fromRight ? 2.0f : -2.0f, pos.y);
            target = pos;
            hasTarget = true;
            m_collider.isStatic = false;
        }

        virtual void update(float deltaTime) {
            splittingCountdown->updateTimer(deltaTime);

            if (!hasTarget) return;

            float dist = glm::length(target - m_position);

            if (dist < 0.1) {
                handleArrived();
            }
            else {
                handleArriving(deltaTime);
            }
        }

    private:
        glm::vec2 target;
        bool hasTarget = false;
        bool isFixed = false;
        std::unique_ptr<Timer> splittingCountdown;
        std::shared_ptr<ObjectPool<Rubbish>> rubbishPool;
        std::shared_ptr<ObjectPool<Block>> blockPool;
        std::shared_ptr<Block> selfPointer;

        float splitStrength;

        void split() {
            if (StatsManager::getInstance().isEasyMode) {
                vanish();
                return;
            }

            if (!m_is_visible) return;

            int parts = 4;

            for (int i = 0; i < parts; i++) {
                std::shared_ptr<Rubbish> tempRubbish = rubbishPool->getInstance();

                if (tempRubbish == nullptr) break;

                tempRubbish->updateType(Rubbish::Effect::NOTHING);
                tempRubbish->show();
                tempRubbish->isPickable = false;
                tempRubbish->resetScale();
                tempRubbish->trashAmount = JUNK_TO_BLOCK / parts;
                tempRubbish->setRubbishPool(rubbishPool);
                tempRubbish->setPosition(m_position);
                tempRubbish->setStatic(true);
                tempRubbish->setSecondGeneration(true);
                tempRubbish->setLandingPosition(m_position + getRandomVector() * splitStrength);

                SceneManager::getInstance().addObject(tempRubbish);
            }

            std::shared_ptr<Block> selfPointer;

            for (std::shared_ptr<Character> object : SceneManager::getInstance()) {
                if (object.get() == this) {
                    selfPointer = std::static_pointer_cast<Block>(object);
                }
            }

            hide();
            setPosition(glm::vec2(2.0f, 2.0f));
            blockPool->returnToPool(selfPointer);
        }

        void vanish() {
            std::shared_ptr<Block> selfPointer;

            for (std::shared_ptr<Character> object : SceneManager::getInstance()) {
                if (object.get() == this) {
                    selfPointer = std::static_pointer_cast<Block>(object);
                }
            }

            hide();
            setPosition(glm::vec2(2.0f, 2.0f));
            blockPool->returnToPool(selfPointer);

            StatsManager::getInstance().collectedBlocks++;
        }

        void handleArrived() {
            hasTarget = false;
        }

        void handleArriving(float deltaTime) {
            m_position = explerpVec2(m_position, target, deltaTime * 1.0f);
        }
};

#endif