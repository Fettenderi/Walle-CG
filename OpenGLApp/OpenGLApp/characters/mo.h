#ifndef MO_H
#define MO_H

#include "../core/shader.h"

#include "../globals/stats_manager.h"

#include "../utils.h"

#include "../core/timer.h"

#include "character.h"

#include <queue>

class Mo : public Character {

    public:

        Mo(std::shared_ptr<ObjectPool<Block>> blockPool, std::shared_ptr<Shader> spriteShader, glm::vec2 position, glm::vec2 scale, const float rotation, const float speed)
            : Character(spriteShader, "assets/textures/mo_atlas.png", CollisionShape(glm::vec2(0.0f, 0.0f), 0.01f, true), position, scale, rotation), maxSpeed(speed), blockPool(blockPool)
        {
            yDebug = 2.76f;

            m_direction = glm::vec2(1.0f, 0.0f);
            block_release_target = glm::vec2(-1.0f, -0.6f);
            StatsManager::getInstance().maxBlockProgress = block_release_target.y;
            camera = SceneManager::getInstance().camera;
            m_speed = maxSpeed;
            player = SceneManager::getInstance().soundManager;
            movingSound = nullptr;

            float temp = to_float(FileManager::getInstance().get(FileManager::CONFIG, "effect_stunned_time"));
            if (temp == 0.0f) {
                temp = 3.0f;
                FileManager::getInstance().set(FileManager::CONFIG, "effect_stunned_time", std::to_string(temp));
            }

            bombTimer = std::make_unique<Timer>(temp, [this] {
                endBombEffect();
                }, false);

            setAtlasGrid(3, 3);
            setTile(0, 1);
            tileAnimationTimer = std::make_unique<Timer>(0.1f, [this] {
                updateTile(startedMoving);
                }, true);
            startedMoving = false;

            bombTimer->pause();
        }

        virtual ~Mo() {
            while (!placedBlocks.empty()) {
                placedBlocks.front().reset();
                placedBlocks.pop();
            }

            pickedBlock.reset();
            camera.reset();
            blockPool.reset();

            bombTimer.release();
            tileAnimationTimer.release();

            if (player != nullptr) {
                player = nullptr;
            }

            if (movingSound != nullptr) {
                movingSound->stop();
                movingSound->drop();
                movingSound = nullptr;
            }
        }

        void setTarget(glm::vec2 pos) {
            if (hasBlock) {
                buffered_target = pos;
                hasBufferedTarget = true;
                return;
            }

            target = pos;
            hasTarget = true;

        }  

        void hitByBomb() {
            bombActive = true;
            bombTimer->resume();
            bombTimer->reset();
        }

        void endBombEffect() {
            bombActive = false;
        }

        void updateTile(bool moving) {
            //printf("timer movimento attivo");
            if (moving) {
                if (currentTile.x == 0) {
                    currentTile.x = 1;
                }
                else if (currentTile.x == 1) {
                    currentTile.x = 2;
                }
            }
            else {
                if (currentTile.x == 2) {
                    currentTile.x = 1;
                }
                else if (currentTile.x == 1) {
                    currentTile.x = 0;
                }
            }
            setTile(currentTile.x, currentTile.y);
        }

        void update(float deltaTime) {
            
            bombTimer->updateTimer(deltaTime);
            tileAnimationTimer->updateTimer(deltaTime);
           if (bombActive) return;
           if (!hasTarget) return;

           m_direction = target - m_position;
           //m_scale.y = glm::abs(m_scale.y) * sign(m_direction.x);
           m_scale.x = glm::abs(m_scale.x) * sign(m_direction.x);

           float dist = glm::length(m_direction);

           if (dist < m_reached_distance) {
               handleArrived();
           } else {
               handleArriving(deltaTime);
           }

           if (glm::length(m_direction) > 0.01f) {
               if (fabs(m_direction.y) > fabs(m_direction.x)) {
                   currentTile.y = (m_direction.y > 0) ? 0 : 1;
               }
               else {
                   currentTile.y = 2;
               }
           }

           setTile(currentTile.x, 2-currentTile.y);
        }

    private:
        glm::vec2 m_direction;
        glm::vec2 m_velocity;
        glm::vec2 target;
        glm::vec2 buffered_target;

        float m_speed;
        float maxSpeed;
        bool hasTarget = false;
        bool hasBufferedTarget = false;
        bool hasBlock = false;
        bool isVertical = false;
        float m_reached_distance = 0.1f;
        float m_collection_distance = 0.3f;

        bool bombActive = false;
        std::unique_ptr<Timer> bombTimer;

        glm::vec2 currentTile = glm::vec2(0,0);
        float animationTimer = 0.0f;
        float hiddenRotation = 0.0f;
        std::unique_ptr<Timer> tileAnimationTimer;
        bool startedMoving = false;

        glm::vec2 block_release_target;
        std::shared_ptr<Block> pickedBlock;
        std::shared_ptr<Camera> camera;
        std::shared_ptr<ObjectPool<Block>> blockPool;
        std::queue<std::shared_ptr<Block>> placedBlocks;

        irrklang::ISoundEngine* player;
        irrklang::ISound* movingSound;

        void handleArrived() {
            startedMoving = false;
            hasTarget = false;

            if (movingSound != nullptr) {
                movingSound->stop();
                movingSound->drop();
                movingSound = nullptr;
            }

            if (hasBlock) {
                releaseBlock();
            } else {
                tryPickingBlock();
            }

            if (hasBufferedTarget) {
                target = buffered_target;
                hasTarget = true;
                hasBufferedTarget = false;
            }
        }

        void handleArriving(float deltaTime) {
            startedMoving = true;
            if (movingSound == nullptr) {
                movingSound = player->play3D("assets/audio/eve_moving.wav", irrklang::vec3df(m_position.x, m_position.y, 0.0f), true, false, true);
            } else {
                movingSound->setPosition(irrklang::vec3df(m_position.x, m_position.y, 0.0f));
            }

            if (!isVertical) {
                if (fabs(target.x - m_position.x) > 0.01f) {
                    m_direction = glm::vec2(target.x - m_position.x, 0.0f); //muovo asse 
                    m_direction = glm::normalize(m_direction);  //or: float dir = (target.x > pos.x) ? 1.0f : -1.0f; m_direction = glm::vec2(dir, 0.0f);
                    m_position += m_direction * m_speed * deltaTime;

                    m_direction.x > 0 ? hiddenRotation = 0.0f : hiddenRotation = 180.0f;
                } else {
                    isVertical = true;
                }
            } else {
                if (fabs(target.y - m_position.y) > 0.01f) {
                    //isVertical = true; 
                    m_direction = glm::vec2(0.0f, target.y - m_position.y); //muovo asse y
                    m_direction = glm::normalize(m_direction); // or: float dir = (target.y > pos.y) ? 1.0f : -1.0f; m_direction = glm::vec2(0.0f, dir);
                    m_position += m_direction * m_speed * deltaTime;

                    m_direction.y > 0 ? hiddenRotation = 270.0f : hiddenRotation = 90.0f;
                } else {
                    isVertical = false;
                }
            }

            if (hasBlock) {
                pickedBlock->setPosition(m_position + m_direction * 0.2f);
                //pickedBlock->setRotation(hiddenRotation);
            }
            
        }

        void tryPickingBlock() {
            for (std::shared_ptr<Character> object : SceneManager::getInstance()) {
                if (std::shared_ptr<Block> block = dynamic_pointer_cast<Block>(object)) {
                    if (block->isPickable && glm::distance(block->getPosition(), m_position) <= m_collection_distance) {
                        pickedBlock = block;
                        pickedBlock->isPickable = false;
                        pickedBlock->setSplittable(false);

                        hasBlock = true;

                        target = getBlockReleasePosition();
                        hasTarget = true;
                        m_speed = maxSpeed * 0.6f;

                        isVertical = false;

                        break;
                    }
                }
            }
        }

        void releaseBlock() {
            pickedBlock->setRotation(0.0f);
            pickedBlock->setPosition(block_release_target);
            player->play3D("assets/audio/mo_release.wav", irrklang::vec3df(block_release_target.x, block_release_target.y, 0.0f), false);

            placedBlocks.push(pickedBlock);

            StatsManager::getInstance().collectedBlocks++;

            pickedBlock = nullptr;
            hasBlock = false;
            m_speed = maxSpeed;
        }

        glm::vec2 getBlockReleasePosition() {
            block_release_target.x += 0.2f;

            if (block_release_target.x >= 0.9f) {
                block_release_target.x = -0.8f;

                block_release_target.y += 0.2f;

                StatsManager::getInstance().maxBlockProgress = fmax(StatsManager::getInstance().maxBlockProgress, block_release_target.y);

                if (placedBlocks.size() >= BLOCK_COLUMNS - 1) {
                    camera->moveTarget(0.2f);
                }

                if (placedBlocks.size() >= 2 * BLOCK_COLUMNS - 1) {
                    for (int i = 0; i < BLOCK_COLUMNS; i++) {
                        std::shared_ptr<Block> freedBlock = placedBlocks.front();
                        placedBlocks.pop();
                        freedBlock->hide();
                        freedBlock->setPosition(glm::vec2(2.0f, 2.0f));
                        blockPool->returnToPool(freedBlock);
                    }
                }
            }

            player->play3D("assets/audio/mo_pickup.wav", irrklang::vec3df(m_position.x, m_position.y, 0.0f), false);

            return block_release_target;
        }
};

#endif