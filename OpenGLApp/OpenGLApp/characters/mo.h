#ifndef MO_H
#define MO_H

#include "../core/shader.h"

#include "../globals/stats_manager.h"

#include "../utils.h"

#include "character.h"

#include <queue>

class Mo : public Character {

    public:

        Mo(std::shared_ptr<ObjectPool<Block>> blockPool, std::shared_ptr<Shader> spriteShader, glm::vec2 position, glm::vec2 scale, const float rotation, const float speed)
            : Character(spriteShader, "assets/textures/mo.png", CollisionShape(glm::vec2(0.0f, 0.0f), 0.01f, false), position, scale, rotation), maxSpeed(speed), blockPool(blockPool)
        {
            m_direction = glm::vec2(1.0f, 0.0f);
            block_release_target = glm::vec2(-1.0f, -0.7f);
            StatsManager::getInstance().maxBlockProgress = block_release_target.y;
            camera = SceneManager::getInstance().camera;
            m_speed = maxSpeed;
            player = SceneManager::getInstance().soundManager;
            movingSound = nullptr;
        }

        virtual ~Mo() {
            while (!placedBlocks.empty()) {
                placedBlocks.front().reset();
                placedBlocks.pop();
            }

            pickedBlock.reset();
            camera.reset();
            blockPool.reset();

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

        void update(float deltaTime) {
           if (!hasTarget) return;

           glm::vec2 pos = getPosition();
           m_direction = target - pos;

           m_rotation = glm::degrees(atan2(-m_direction.y, m_direction.x));
           m_scale.y = glm::abs(m_scale.y) * sign(m_direction.x);

           float dist = glm::length(m_direction);

           if (dist < m_reached_distance) {
               hasTarget = false;

               if (movingSound != nullptr) {
                   movingSound->stop();
                   movingSound->drop();
                   movingSound = nullptr;
               }

               if (hasBlock) {
                   releaseBlock();
               }
               else {
                   tryPickingBlock();
               }

               if (hasBufferedTarget) {
                   target = buffered_target;
                   hasTarget = true;
                   hasBufferedTarget = false;
               }
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

               if (hasBlock) {
                   pickedBlock->setPosition(m_position + m_direction * 0.2f);
                   pickedBlock->setRotation(m_rotation);
               }
           }
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
        float m_reached_distance = 0.2f;
        float m_collection_distance = 0.3f;

        glm::vec2 block_release_target;
        std::shared_ptr<Block> pickedBlock;
        std::shared_ptr<Camera> camera;
        std::shared_ptr<ObjectPool<Block>> blockPool;
        std::queue<std::shared_ptr<Block>> placedBlocks;

        irrklang::ISoundEngine* player;
        irrklang::ISound* movingSound;

        void tryPickingBlock() {
            for (std::shared_ptr<Character> object : SceneManager::getInstance()) {
                if (std::shared_ptr<Block> block = dynamic_pointer_cast<Block>(object)) {
                    if (block->isPickable && glm::distance(block->getPosition(), m_position) <= m_collection_distance) {
                        pickedBlock = block;
                        pickedBlock->isPickable = false;

                        hasBlock = true;

                        target = getBlockReleasePosition();
                        hasTarget = true;
                        m_speed = maxSpeed * 0.6f;

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

                if (placedBlocks.size() >= 5 * BLOCK_COLUMNS - 1) {
                   for (int i = 0; i < BLOCK_COLUMNS; i++) {
                       std::shared_ptr<Block> freedBlock = placedBlocks.front();
                       placedBlocks.pop();
                       freedBlock->setPosition(glm::vec2(2.0f, 2.0f));
                       freedBlock->hide();
                       blockPool->returnToPool(freedBlock);
                   }
                }
            }

            player->play3D("assets/audio/mo_pickup.wav", irrklang::vec3df(m_position.x, m_position.y, 0.0f), false);

            camera->moveTarget(0.18f / 9.0f);

            return block_release_target;
        }
};

#endif