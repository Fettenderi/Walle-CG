#ifndef RUBBISH_H
#define RUBBISH_H

#include "../core/shader.h"
#include "../core/timer.h"
#include "../core/pool.h"
#include "../utils.h"

#include "../globals/stats_manager.h"
#include "../globals/scene_manager.h"

#include "character.h"

class Rubbish : public Character {

    public:
        bool isPickable = false;
        int trashAmount;

        Rubbish(std::shared_ptr<Shader> spriteShader, glm::vec2 position, glm::vec2 scale)
            : Character(spriteShader, "assets/textures/rubbish.png", CollisionShape(glm::vec2(0.0f, 0.0f), 0.1f, true), position, scale, 0.0f)
        {
            m_is_visible = false;

            splitStrength = 0.3f;
            splitTime = 8.0f;

            splittingCountdown = std::make_unique<Timer>(getNormalRandomClamped(splitTime, 1.0f), [this] {
                split();
                }, false);

            splittingCountdown->pause();
        }

        ~Rubbish() {
            rubbishPool.reset();

            splittingCountdown.release();
        }

        void setPosition(glm::vec2 position) {
            m_position = position;
        }

        void setLandingPosition(glm::vec2 pos) {
            hasTarget = true;
            target = clampInCamera(pos, SceneManager::getInstance().camera->getPosition2D(), StatsManager::getInstance().maxBlockProgress);
        }

        void setRubbishPool(std::shared_ptr<ObjectPool<Rubbish>> pool) {
            rubbishPool = pool;
        }

        void setStatic(bool value) {
            m_collider.isStatic = value;
        }

        void setSecondGeneration(bool value) {
            if (value) {
                splittingCountdown->changeDuration(getNormalRandomClamped(splitTime * 2.0f, 1.0f));
            } else {
                splittingCountdown->changeDuration(getNormalRandomClamped(splitTime, 1.0f));
            }
        }

        virtual void update(float deltaTime) {
            splittingCountdown->updateTimer(deltaTime);

            if (abs(m_uniform_scale - targetUniformScale) > 0.01f) {
                m_uniform_scale = explerp(m_uniform_scale, targetUniformScale, deltaTime * 1.5f);
            }

            if (hasTarget) {
                handleTarget(deltaTime);
                return;
            }
        }

    private:
        glm::vec2 target;
        bool hasTarget = false;
        std::unique_ptr<Timer> splittingCountdown;
        std::shared_ptr<ObjectPool<Rubbish>> rubbishPool;

        float splitTime;
        float splitStrength;
        float targetUniformScale = 1.0f;

        void split() {
            if (!m_is_visible) return;
            if (m_uniform_scale < 1.0f) {

                trashAmount += getNextRandomIntRange(1, 3);
                targetUniformScale = 0.9f;
                return;
            }

            int parts = 4;

            for (int i = 0; i < parts - 1; i++) {
                std::shared_ptr<Rubbish> tempRubbish = rubbishPool->getInstance();

                if (tempRubbish == nullptr) break;

                tempRubbish->show();
                tempRubbish->isPickable = false;
                tempRubbish->trashAmount = ceil(trashAmount / parts);
                tempRubbish->setRubbishPool(rubbishPool);
                tempRubbish->setPosition(m_position);
                tempRubbish->setStatic(true);
                tempRubbish->setSecondGeneration(true);
                tempRubbish->targetUniformScale = m_uniform_scale - 0.1f * parts;
                tempRubbish->setLandingPosition(m_position + getRandomVector() * splitStrength);

                SceneManager::getInstance().addObject(tempRubbish);
            }
            
            isPickable = false;
            trashAmount = ceil(trashAmount / parts);
            targetUniformScale = m_uniform_scale - 0.1f * parts;
            setStatic(true);
            setSecondGeneration(true);
            setLandingPosition(m_position + getRandomVector() * splitStrength);
        }

        void handleTarget(float deltaTime) {
            float dist = glm::length(target - m_position);

            if (dist < 0.1) {
                handleArrived();
            }
            else {
                handleArriving(deltaTime);
            }
        }

        void handleArrived() {
            hasTarget = false;
            isPickable = true;
            m_collider.isStatic = false;

            splittingCountdown->reset();
            splittingCountdown->resume();

            StatsManager::getInstance().currentRubbish++;
        }

        void handleArriving(float deltaTime) {
            m_position = explerpVec2(m_position, target, deltaTime * 2.0f);
        }
};

#endif