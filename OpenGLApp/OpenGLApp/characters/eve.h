#ifndef EVE_H
#define EVE_H

#include "../core/shader.h"
#include "../core/timer.h"

#include "../globals/scene_manager.h"

#include "../utils.h"

#include "rubbish.h"
#include "character.h"

#include <memory>

class Eve : public Character {

    public:

        Eve(std::shared_ptr<ObjectPool<Rubbish>> rubbishPool, std::shared_ptr<Shader> spriteShader, glm::vec2 position, glm::vec2 scale, const float speed)
            : Character(spriteShader, "assets/textures/eve.png", position, scale, 0.0f), m_speed(speed), rubbishPool(rubbishPool)
        {
            m_direction = glm::vec2(1.0f, 0.0f);

            cooldownTimer = std::make_unique<Timer>(getNextRandomRange(0.5f, 0.6f), [this] { //4 6
                    tryGettingRubbish();
                } , false);
        }


        void update(float deltaTime) {
            cooldownTimer->updateTimer(deltaTime);

            if (!hasTarget) return;

            m_direction = target - m_position;

            m_rotation = glm::degrees(atan2(-m_direction.y, m_direction.x));
            m_scale.y = glm::abs(m_scale.y) * sign(m_direction.x);

            float dist = glm::length(m_direction);

            if (dist < 0.01f) {
                if (!hasRubbish) {
                    hasTarget = false;

                    cooldownTimer->changeDuration(getNextRandomRange(0.5f, 0.6f)); // 4, 6
                    cooldownTimer->resume();
                    cooldownTimer->reset();

                    return;
                }
                
                target = m_prev_position;

                pickedRubbish->isPickable = true;
                hasRubbish = false;
                pickedRubbish = nullptr;
            }
            else {
                m_direction = glm::normalize(m_direction);
                m_position += m_direction * m_speed * deltaTime;

                if (hasRubbish)
                    pickedRubbish->setPosition(m_position + m_direction * 0.2f);
            }
        }

        void free() {}

  

    private:
        glm::vec2 m_prev_position;

        glm::vec2 m_direction;
        glm::vec2 target;

        float m_speed;
        bool hasTarget = false;
        bool hasRubbish = false;

        std::shared_ptr<Rubbish> pickedRubbish;
        std::unique_ptr<Timer> cooldownTimer;
        std::shared_ptr<ObjectPool<Rubbish>> rubbishPool;

        void tryGettingRubbish() {
            setTarget(glm::vec2(getNextRandom(), getNextRandom()) * 0.8f - SceneManager::getInstance().camera->getPosition2D());
        }

        void setTarget(glm::vec2 pos) {
            pickedRubbish = rubbishPool->getInstance();

            if (pickedRubbish == nullptr) return;

            target = pos;

            m_position = glm::vec2(sign(getNextRandom()) * 2.0f, pos.y);
            m_prev_position = m_position;


            pickedRubbish->show();
            pickedRubbish->isPickable = false;

            SceneManager::getInstance().addObject(pickedRubbish);

            hasTarget = true;
            hasRubbish = true;
        }

};

#endif