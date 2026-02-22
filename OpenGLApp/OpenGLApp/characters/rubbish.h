#ifndef RUBBISH_H
#define RUBBISH_H

#include "../core/shader.h"
#include "../core/timer.h"
#include "../core/pool.h"

#include "../utils.h"

#include "../globals/scene_manager.h"
#include "../globals/stats_manager.h"
#include "../globals/file_manager.h"

#include "character.h"

class Rubbish : public Character {

    public:
        bool isPickable = false;
        bool isAttracted = false;
        int trashAmount;
        int rubbishType = 0;

        Rubbish(std::shared_ptr<Shader> spriteShader, glm::vec2 position, glm::vec2 scale)
            : Character(spriteShader, "assets/textures/rubbish_atlas.png", CollisionShape(glm::vec2(0.0f, 0.0f), 0.1f, true), position, scale, 0.0f)
        {
            m_is_visible = false;

            splitStrength = to_float(FileManager::getInstance().get(FileManager::CONFIG, "rubbish_split_strength"));
            if (splitStrength == 0.0f) {
                splitStrength = 0.3f;
                FileManager::getInstance().set(FileManager::CONFIG, "rubbish_split_strength", std::to_string(splitStrength));
            }

            splitTime = to_float(FileManager::getInstance().get(FileManager::CONFIG, "rubbish_split_time"));
            if (splitTime == 0.0f) {
                splitTime = 8.0f;
                FileManager::getInstance().set(FileManager::CONFIG, "rubbish_split_time", std::to_string(splitTime));
            }

            splittingCountdown = std::make_unique<Timer>(getNormalRandomClamped(splitTime, 1.0f), [this] {
                split();
                }, false);

            splittingCountdown->pause();

            setAtlasGrid(5, 3);
            setTile(0, 2);
            //effects = std::make_unique<RubbishEffects>(this);

            maxScale = getScale();

            float temp = to_float(FileManager::getInstance().get(FileManager::CONFIG, "bomb_time"));
            if (temp == 0.0f) {
                temp = 5.0f;
                FileManager::getInstance().set(FileManager::CONFIG, "bomb_time", std::to_string(temp));
            }

            bombTimer = std::make_unique<Timer>(getNormalRandomClamped(temp, 1.0f), [this] {
                explodeBomb();
                }, false);

            temp = to_float(FileManager::getInstance().get(FileManager::CONFIG, "fire_ext_on_time"));
            if (temp == 0.0f) {
                temp = 2.0f;
                FileManager::getInstance().set(FileManager::CONFIG, "fire_ext_on_time", std::to_string(temp));
            }

            fireExtOnTimer = std::make_unique<Timer>(getNormalRandomClamped(temp, 1.0f), [this] {
                resetOffTimer();
                }, false);

            temp = to_float(FileManager::getInstance().get(FileManager::CONFIG, "fire_ext_off_time"));
            if (temp == 0.0f) {
                temp = 2.0f;
                FileManager::getInstance().set(FileManager::CONFIG, "fire_ext_off_time", std::to_string(temp));
            }

            fireExtOffTimer = std::make_unique<Timer>(getNormalRandomClamped(temp, 1.0f), [this] {
                resetOnTimer();
                }, false);

            fireExtOnTimer->pause();
            fireExtOffTimer->pause();
            bombTimer->pause();

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

        void moveToWalle(glm::vec2 pos) {
            hasTarget = true;
            target = pos;
        }

        void stopMoving() {
            hasTarget = false;
        }

        void updateType(int type) {
            rubbishType = type;
            /*std::string path = "assets/textures/rubbish" + std::to_string(type) + ".png";
            const char* cpath = path.c_str();

            updateTexture(cpath);*/
            setTile(type, 2);

            printf("nuovo tipo di rubbish: %i\n", rubbishType);

            if (type == 3) {
                fireExtOffTimer->resume();
            }
        }

        void resetOnTimer() {
            fireExtOnTimer->resume();
            fireExtOnTimer->reset();
            fireExtActive = true;
        }

        void resetOffTimer() {
            fireExtOffTimer->reset();
            fireExtActive = false;
            setScale(maxScale);
        }

        int fireExtState() {
            if (rubbishType != 3 || !fireExtActive)
                return 0;
            else return 3;
        }

        void activateBomb() {
            bombTimer->resume();
            bombTimer->reset();
            bombActive = true;
        }

        void explodeBomb() {
            std::vector<std::shared_ptr<Character>> objects = SceneManager::getInstance().getObjects();
            for (const std::shared_ptr<Character>& obj : objects) {
                obj->hitByBomb(); 
            }
            printf("\n\n\nBOMBA ESPLOSA");
        }

        int executeEffect() {
            switch (rubbishType) {
            case 0:
                return 0;
            case 1: //magnete
                return 1;
            case 2:
                return 2;
            case 3: //estintore
                return fireExtState(); //controllo lo stato attuale per ritornare 0 se non attivo o 3 se attivo
            case 4: //bomba 
                activateBomb();
                return 0;

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

            if (rubbishType == 3) {
                fireExtOnTimer->updateTimer(deltaTime);
                fireExtOffTimer->updateTimer(deltaTime);

                if (fireExtActive) {
                    currentScale = glm::vec2(abs(sin(fireExtOnTimer->getElapsed() * 5.0f)) * 0.15 + maxScale.x * 0.8f, abs(cos(fireExtOnTimer->getElapsed() * 6.0f)) * 0.15 + maxScale.y * 0.8f);
                    setScale(currentScale);
                    return;
                }

            }

            if (rubbishType == 4) {
                if (!m_is_visible)
                    bombTimer->pause();
                bombTimer->updateTimer(deltaTime);
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

        std::unique_ptr<Timer> bombTimer;
        bool bombActive = false;
        //std::unique_ptr<RubbishEffects> effects;

        std::unique_ptr<Timer> fireExtOnTimer;
        std::unique_ptr<Timer> fireExtOffTimer;
        bool fireExtActive = false;

        glm::vec2 maxScale;
        glm::vec2 currentScale;

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