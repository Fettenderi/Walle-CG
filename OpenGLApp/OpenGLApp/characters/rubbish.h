#ifndef RUBBISH_H
#define RUBBISH_H

#include "../core/shader.h"
#include "../core/timer.h"
#include "../utils.h"

#include "../globals/stats_manager.h"

#include "character.h"

class Rubbish : public Character {

    public:
        bool isPickable = false;

        Rubbish(std::shared_ptr<Shader> spriteShader, glm::vec2 position, glm::vec2 scale)
            : Character(spriteShader, "assets/textures/rubbish.png", CollisionShape(glm::vec2(0.0f, 0.0f), 0.1f, true), position, scale, 0.0f)
        {
            m_is_visible = false;
        }

        void setPosition(glm::vec2 position) {
            m_position = position;
        }

        void setLandingPosition(glm::vec2 pos) {
            hasTarget = true;
            target = pos;
        }

        virtual void update(float deltaTime) {
            if (hasTarget) {
                handleTarget(deltaTime);
                return;
            }

            if (splittingCountdown != nullptr)
                splittingCountdown->updateTimer(deltaTime);
        }

        int trashAmount;


    private:
        glm::vec2 target;
        bool hasTarget = false;
        std::unique_ptr<Timer> splittingCountdown;

        void split() {

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
            StatsManager::getInstance().currentRubbish++;

            splittingCountdown = std::make_unique<Timer>(getNormalRandomClamped(8.0f, 1.0f), [this] {
                split();
                }, false);
        }

        void handleArriving(float deltaTime) {
            m_position = explerpVec2(m_position, target, deltaTime * 2.0f);
        }
};

#endif