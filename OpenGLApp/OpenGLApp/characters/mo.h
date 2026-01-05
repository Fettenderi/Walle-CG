#ifndef MO_H
#define MO_H

#include "../core/shader.h"

#include "../utils.h"

#include "character.h"

#include <queue>

class Mo : public Character {

    public:

        Mo(std::shared_ptr<ObjectPool<Block>> blockPool, std::shared_ptr<Shader> spriteShader, glm::vec2 position, glm::vec2 scale, const float rotation, const float speed)
            : Character(spriteShader, "assets/textures/mo.png", position, scale, rotation), m_speed(speed), blockPool(blockPool)
        {
            m_direction = glm::vec2(1.0f, 0.0f);
            block_release_target = glm::vec2(-1.0f, -0.7f);
            camera = SceneManager::getInstance().camera;
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
            else {
                m_direction = glm::normalize(m_direction);
                m_position += m_direction * m_speed * deltaTime;

                if (hasBlock) {
                    pickedBlock->setPosition(m_position + m_direction * 0.2f);
                    pickedBlock->setRotation(m_rotation);
                }
            }
        }

        void free() {}

  

    private:
        glm::vec2 m_direction;
        glm::vec2 m_velocity;
        glm::vec2 target;
        glm::vec2 buffered_target;

        float m_speed;
        bool hasTarget = false;
        bool hasBufferedTarget = false;
        bool hasBlock = false;
        float m_reached_distance = 0.1f;
        float m_collection_distance = 0.2f;

        glm::vec2 block_release_target;
        std::shared_ptr<Block> pickedBlock;
        std::shared_ptr<Camera> camera;
        std::shared_ptr<ObjectPool<Block>> blockPool;
        std::queue<std::shared_ptr<Block>> placedBlocks;

        void tryPickingBlock() {
            for (std::shared_ptr<Character> object : SceneManager::getInstance()) {
                if (std::shared_ptr<Block> block = dynamic_pointer_cast<Block>(object)) {
                    if (block->isPickable && glm::distance(block->getPosition(), m_position) <= m_collection_distance) {
                        pickedBlock = block;
                        pickedBlock->isPickable = false;

                        hasBlock = true;

                        target = getBlockReleasePosition();
                        hasTarget = true;

                        break;
                    }
                }
            }
        }

        void releaseBlock() {
            pickedBlock->setRotation(0.0f);
            pickedBlock->setPosition(block_release_target);

            placedBlocks.push(pickedBlock);

            pickedBlock = nullptr;
            hasBlock = false;
        }

        glm::vec2 getBlockReleasePosition() {
            block_release_target.x += 0.2f;

            if (block_release_target.x >= 0.9f) {
                block_release_target.x = -0.8f;

                block_release_target.y += 0.2f;

                if (placedBlocks.size() >= 17) {
                   for (int i = 0; i < 9; i++) {
                       std::shared_ptr<Block> freedBlock = placedBlocks.front();
                       placedBlocks.pop();
                       freedBlock->setPosition(glm::vec2(2.0f, 2.0f));
                       freedBlock->hide();
                       blockPool->returnToPool(freedBlock);
                   }
                }

            }
            camera->setTarget(camera->getY() - 0.033f);

            return block_release_target;
        }
};

#endif