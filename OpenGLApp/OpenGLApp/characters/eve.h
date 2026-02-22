#ifndef EVE_H
#define EVE_H

#include "../core/shader.h"
#include "../core/timer.h"

#include "../globals/scene_manager.h"
#include "../globals/stats_manager.h"
#include "../globals/file_manager.h"

#include "../utils.h"

#include "rubbish.h"
#include "character.h"

#include <memory>
#include <list>
#include <format>
#include <iostream>
#include <string>

class Eve : public Character {

    public:

        Eve(std::shared_ptr<ObjectPool<Rubbish>> rubbishPool, std::shared_ptr<Shader> spriteShader, glm::vec2 position, glm::vec2 scale, const float speed)
            : Character(spriteShader, "assets/textures/eve.png", CollisionShape(glm::vec2(0.0f, 0.0f), 0.1f, true), position, scale, 0.0f), m_speed(speed), rubbishPool(rubbishPool)
        {
            m_direction = glm::vec2(1.0f, 0.0f);

            timeBetweenDelivery = to_float(FileManager::getInstance().get(FileManager::CONFIG, "eve_time_betw_delivery"));
            if (timeBetweenDelivery == 0.0f) {
                timeBetweenDelivery = 1.0f;
                FileManager::getInstance().set(FileManager::CONFIG, "eve_time_betw_delivery", std::to_string(timeBetweenDelivery));
            }

            timeBetweenDeliveryDeviation = to_float(FileManager::getInstance().get(FileManager::CONFIG, "eve_time_betw_delivery_std"));
            if (timeBetweenDeliveryDeviation == 0.0f) {
                timeBetweenDeliveryDeviation = 0.5f;
                FileManager::getInstance().set(FileManager::CONFIG, "eve_time_betw_delivery_std", std::to_string(timeBetweenDeliveryDeviation));
            }

            deliveryAmount = to_int(FileManager::getInstance().get(FileManager::CONFIG, "eve_delivery_amount"));
            if (deliveryAmount == 0) {
                deliveryAmount = 3;
                FileManager::getInstance().set(FileManager::CONFIG, "eve_delivery_amount", std::to_string(deliveryAmount));
            }

            float throwCooldownTime = 0.5f;

            player = SceneManager::getInstance().soundManager;
            movingSound = nullptr;

            cooldownTimer = std::make_unique<Timer>(getNormalRandomClamped(timeBetweenDelivery, timeBetweenDeliveryDeviation), [this] {
                    tryGettingRubbish();
                } , false);

            throwCooldownTimer = std::make_unique<Timer>(throwCooldownTime, [this] {
                throwRubbish();
                }, false);

            throwCooldownTimer->pause();
        }

        virtual ~Eve() {
            for (std::shared_ptr<Rubbish> rubbish : pickedRubbish) {
                rubbish.reset();
            }

            pickedRubbish.clear();
            rubbishPositions.clear();

            cooldownTimer.release();
            throwCooldownTimer.release();

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
            throwCooldownTimer->updateTimer(deltaTime);

            if (currentState != MOVING && currentState != RETURNING) return;

            m_direction = target - m_position;

            m_rotation = glm::degrees(atan2(-m_direction.y, m_direction.x));
            m_scale.y = glm::abs(m_scale.y) * sign(m_direction.x);

            float dist = glm::length(m_direction);

            if (dist < 0.01f)
                handleArrived();
            else
                handleMovingToTarget(deltaTime);
        }

    private:
        enum State {
            IDLE, MOVING, THROWING, RETURNING
        };

        glm::vec2 m_prev_position;

        glm::vec2 m_direction;
        glm::vec2 target;

        glm::vec2 frontPosition;

        float m_speed;
        State currentState = IDLE;

        std::list<std::shared_ptr<Rubbish>> pickedRubbish;
        std::list<glm::vec2> rubbishPositions;
        std::unique_ptr<Timer> cooldownTimer;
        std::unique_ptr<Timer> throwCooldownTimer;
        std::shared_ptr<ObjectPool<Rubbish>> rubbishPool;
        irrklang::ISoundEngine* player;
        irrklang::ISound* movingSound;

        float timeBetweenDelivery;
        float timeBetweenDeliveryDeviation;
        int deliveryAmount;

        void handleArrived() {
            if (currentState == MOVING) {
                currentState = THROWING;

                throwCooldownTimer->resume();
                throwCooldownTimer->reset();
            }
            else if (currentState == RETURNING) {
                currentState = IDLE;

                cooldownTimer->changeDuration(getNormalRandomClamped(timeBetweenDelivery, timeBetweenDeliveryDeviation));

                cooldownTimer->resume();
                cooldownTimer->reset();
            }
        }

        void throwRubbish() {
            if (pickedRubbish.empty()) {
                handleFinishedThrowing();
                return;
            }

            std::shared_ptr<Rubbish> rubbish = pickedRubbish.front();
            glm::vec2 position = rubbishPositions.front();

            pickedRubbish.pop_front();
            rubbishPositions.pop_front();

            rubbish->setPosition(frontPosition);
            rubbish->setLandingPosition(position);

            throwCooldownTimer->resume();
            throwCooldownTimer->reset();

            if (!pickedRubbish.empty()) {
                rubbish = pickedRubbish.front();
                rubbish->setPosition(frontPosition);
            }
        }

        void handleFinishedThrowing() {
            currentState = RETURNING;

            target = m_prev_position;

            if (movingSound != nullptr) {
                movingSound->stop();
                movingSound->drop();
                movingSound = nullptr;
            }
        }

        void handleMovingToTarget(float deltaTime) {
            if (movingSound == nullptr) {
                movingSound = player->play3D("assets/audio/eve_moving.wav", irrklang::vec3df(m_position.x, m_position.y, 0.0f), true, false, true);
            }
            else {
                movingSound->setPosition(irrklang::vec3df(m_position.x, m_position.y, 0.0f));
            }

            m_direction = glm::normalize(m_direction);
            m_position += m_direction * m_speed * deltaTime;

            frontPosition = m_position + m_direction * 0.2f;

            if (!pickedRubbish.empty()) {
                std::shared_ptr<Rubbish> rubbish = pickedRubbish.front();
                rubbish->setPosition(frontPosition);
            }
        }

        int normalRubbishLeft = 1;
        int lastRubbishType = 0;

        //return 0 se normale o 1-4 per spazzatura speciale
        int updateRubbishQueue() {

            //se ho ancora spazzatura normale
            if (normalRubbishLeft > 0) {
                //printf("spawn rubbish normale\n");
                normalRubbishLeft--;
                return 0;
            }

            //printf("finita la rubbish normale\n");

            //reset counter
            normalRubbishLeft = getNextRandomIntRange(1, 2);

            //speciale diverso dal precedente
            int newRubbishType;
            do {
                newRubbishType = getNextRandomIntRange(1, 4);
            } while (newRubbishType == lastRubbishType);

            lastRubbishType = newRubbishType;

            printf("spawn rubbish speciale tipo %d\n", newRubbishType);
            return newRubbishType;
        }

        void tryGettingRubbish() {
            int actualDelivery = getNextRandomIntRange(std::max(deliveryAmount - 1, 1), deliveryAmount + 1);
            bool foundRubbish = false;
            std::shared_ptr<Rubbish> tempRubbish;
            glm::vec2 midPoint = glm::vec2(0.0f, 0.0f);

            for (int i = 0; i < actualDelivery; i++) {
                tempRubbish = rubbishPool->getInstance();

                if (tempRubbish == nullptr) break;

                foundRubbish = true;

                tempRubbish->show();
                tempRubbish->isPickable = false;
                tempRubbish->trashAmount = getNextRandomIntRange(2, JUNK_TO_BLOCK / 3 + 1);
                tempRubbish->setUniformScale(1.0f);
                //tempRubbish->setScale(tempRubbish->getMaxScale());
                tempRubbish->setRubbishPool(rubbishPool);
                tempRubbish->setStatic(true);
                tempRubbish->setSecondGeneration(false);

                tempRubbish->updateType(updateRubbishQueue());
                if (lastRubbishType == 4)
                    tempRubbish->activateBomb();
                //printf("update rubbish finito su eve\n");

                pickedRubbish.push_back(tempRubbish);
                glm::vec2 tempPosition = getRandomPosition(SceneManager::getInstance().camera->getPosition2D(), StatsManager::getInstance().maxBlockProgress);
                rubbishPositions.push_back(tempPosition);

                midPoint += tempPosition;

                SceneManager::getInstance().addObject(tempRubbish);
            }

            if (!foundRubbish) return;

            midPoint /= actualDelivery;

            target = glm::vec2(sign(midPoint.x) * 0.8f, midPoint.y);

            m_position = glm::vec2(sign(midPoint.x) * 2.0f, midPoint.y);
            m_prev_position = m_position;

            currentState = MOVING;
        }

};

#endif