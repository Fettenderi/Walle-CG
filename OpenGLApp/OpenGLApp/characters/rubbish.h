#ifndef RUBBISH_H
#define RUBBISH_H

#include "../core/shader.h"
#include "../utils.h"
#include "../core/timer.h"

#include "../globals/scene_manager.h"

#include "character.h"
#include "rubbish_effects.h"

class Rubbish : public Character {

    public:
        bool isPickable = false;
        bool isAttracted = false;
        int rubbishType = 0;

        Rubbish(std::shared_ptr<Shader> spriteShader, glm::vec2 position, glm::vec2 scale)
            : Character(spriteShader, "assets/textures/rubbish_atlas.png", CollisionShape(glm::vec2(0.0f, 0.0f), 0.1f, true), position, scale, 0.0f)
        {
            m_is_visible = false;
            
            setAtlasGrid(5, 3);
            setTile(0, 2);
            //effects = std::make_unique<RubbishEffects>(this);


            maxScale = getScale();

            bombTimer = std::make_unique<Timer>(5.0f, [this] {
                explodeBomb();
                }, false);
            fireExtOnTimer = std::make_unique<Timer>(2.0f, [this] {
                resetOffTimer();
                }, false);
            fireExtOffTimer = std::make_unique<Timer>(2.0f, [this] {
                resetOnTimer();
                }, false);

            fireExtOnTimer->pause();
            fireExtOffTimer->pause();
            bombTimer->pause();
        }

        

        void setPosition(glm::vec2 position) {
            m_position = position;
        }

        void getBlownTo(glm::vec2 pos, bool fromRight) {
            m_position = glm::vec2(fromRight ? 2.0f : -2.0f, pos.y);
            hasTarget = true;
            target = pos;
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
            

            if (!hasTarget) return;

            float dist = glm::length(target - m_position);

            //if (dist > 0.5) return; //viene attratta solo la spazzatura abbastanza vicina a walle, non tutta quella a schermo

            if (dist < 0.1) {
                hasTarget = false;
                isPickable = true;
            }
            else {
                m_position = explerpVec2(m_position, target, deltaTime * 1.0f);
            }
        }

        int trashAmount;


    private:
        glm::vec2 target;
        bool hasTarget = false;
        std::unique_ptr<Timer> bombTimer;
        bool bombActive = false;
        //std::unique_ptr<RubbishEffects> effects;




        std::unique_ptr<Timer> fireExtOnTimer;
        std::unique_ptr<Timer> fireExtOffTimer;
        bool fireExtActive = false;

        glm::vec2 maxScale;
        glm::vec2 currentScale;
};

#endif