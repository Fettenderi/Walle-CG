#ifndef EVE_H
#define EVE_H

#include "../core/shader.h"
#include "../core/timer.h"

#include "../globals/scene_manager.h"
#include "../globals/stats_manager.h"

#include "../utils.h"

#include "rubbish.h"
#include "character.h"

#include <memory>
#include <list>

class Eve : public Character {

    public:

        Eve(std::shared_ptr<ObjectPool<Rubbish>> rubbishPool, std::shared_ptr<Shader> spriteShader, glm::vec2 position, glm::vec2 scale, const float speed)
            : Character(spriteShader, "assets/textures/eve.png", CollisionShape(glm::vec2(0.0f, 0.0f), 0.1f, true), position, scale, 0.0f), m_speed(speed), rubbishPool(rubbishPool)
        {
            m_direction = glm::vec2(1.0f, 0.0f);

            timeBetweenDelivery = 1.0f;
            timeBetweenDeliveryDeviation = 0.5f;
            deliveryAmount = 1;

            player = SceneManager::getInstance().soundManager;
            movingSound = nullptr;

            cooldownTimer = std::make_unique<Timer>(getNormalRandomClamped(timeBetweenDelivery, timeBetweenDeliveryDeviation), [this] {
                    tryGettingRubbish();
                } , false);
        }

        virtual ~Eve() {
            for (std::shared_ptr<Rubbish> rubbish : pickedRubbish) {
                rubbish.reset();
            }

            pickedRubbish.clear();
            rubbishOffsets.clear();

            cooldownTimer.release();

            rubbishPool.reset();

            if (player != nullptr) {
                player = nullptr;
            }

            if (movingSound != nullptr) {
                movingSound->stop();
                movingSound->drop();
                movingSound = nullptr;
            }
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

                    cooldownTimer->changeDuration(getNormalRandomClamped(timeBetweenDelivery, timeBetweenDeliveryDeviation));

                    cooldownTimer->resume();
                    cooldownTimer->reset();

                    if (movingSound != nullptr) {
                        movingSound->stop();
                        movingSound->drop();
                        movingSound = nullptr;
                    }

                    return;
                }
                
                target = m_prev_position;

                int deposited = 0;
                for (std::shared_ptr<Rubbish> tempRubbish : pickedRubbish) {
                    tempRubbish->isPickable = true;
                    deposited++;
                }

                StatsManager::getInstance().currentRubbish += deposited;

                pickedRubbish.clear();
                rubbishOffsets.clear();

                hasRubbish = false;
            }
            else {
                if (movingSound == nullptr) {
                    movingSound = player->play3D("assets/audio/eve_moving.wav", irrklang::vec3df(m_position.x, m_position.y, 0.0f), true, false, true);
                }
                else {
                    movingSound->setPosition(irrklang::vec3df(m_position.x, m_position.y, 0.0f));
                }

                m_direction = glm::normalize(m_direction);
                m_position += m_direction * m_speed * deltaTime;

                auto tempRubbish = pickedRubbish.begin();
                auto offset = rubbishOffsets.begin();

                for (; tempRubbish != pickedRubbish.end() && offset != rubbishOffsets.end(); ++tempRubbish, ++offset) {
                    //int offset;
                    //if (i % 2 == 0) {
                      //  offset = -i / 2;
                    //} else {
                      //  offset = (int)ceil(((float)i) / 2.0f);
                    //}

                    //tempRubbish->setPosition(glm::vec2(0.0f, offset * 0.1f) + m_position + m_direction * 0.2f);
                    //i++;

                    (*tempRubbish)->setPosition(*offset + m_position + m_direction * 0.2f);
                }
            }
        }

    private:
        glm::vec2 m_prev_position;

        glm::vec2 m_direction;
        glm::vec2 target;

        float m_speed;
        bool hasTarget = false;
        bool hasRubbish = false;

        std::list<std::shared_ptr<Rubbish>> pickedRubbish;
        std::list<glm::vec2> rubbishOffsets;
        std::unique_ptr<Timer> cooldownTimer;
        std::shared_ptr<ObjectPool<Rubbish>> rubbishPool;
        irrklang::ISoundEngine* player;
        irrklang::ISound* movingSound;

        float timeBetweenDelivery;
        float timeBetweenDeliveryDeviation;
        int deliveryAmount;

        void tryGettingRubbish() {
            setTarget(getRandomPosition(SceneManager::getInstance().camera->getPosition2D(), StatsManager::getInstance().maxBlockProgress));
        }

        void setTarget(glm::vec2 pos) {
            int actualDelivery = getNextRandomIntRange(std::max(deliveryAmount - 1, 1), deliveryAmount + 1);
            bool foundRubbish = false;
            std::shared_ptr<Rubbish> tempRubbish;

            for (int i = 0; i < actualDelivery; i++) {
                tempRubbish = rubbishPool->getInstance();

                if (tempRubbish == nullptr) break;

                foundRubbish = true;

                tempRubbish->show();
                tempRubbish->isPickable = false;
                tempRubbish->trashAmount = getNextRandomIntRange(2, JUNK_TO_BLOCK / 3 + 1);

                pickedRubbish.push_back(tempRubbish);
                rubbishOffsets.push_back(glm::normalize(getRandomVector()) * (((float)i) * 0.2f));

                SceneManager::getInstance().addObject(tempRubbish);
            }

            if (!foundRubbish) return;
            
            target = pos;

            m_position = glm::vec2(sign(getNextRandom()) * 2.0f, pos.y);
            m_prev_position = m_position;

            hasTarget = true;
            hasRubbish = true;
        }

};

#endif