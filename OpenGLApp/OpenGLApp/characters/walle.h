#ifndef WALLE_H
#define WALLE_H

#include "../utils.h"

#include "character.h"
#include "rubbish.h"
#include "block.h"

#include "../globals/stats_manager.h"
#include "../globals/scene_manager.h"

#include "../core/light.h"
#include "../core/timer.h"

#include <cmath>

class Walle : public Character {

    public:
        Walle(std::shared_ptr<ObjectPool<Rubbish>> rubbishPool, std::shared_ptr<ObjectPool<Block>> blockPool, std::shared_ptr<Shader> spriteShader, glm::vec2 position, glm::vec2 scale, const float rotation, const float speed)
            : Character(spriteShader, "assets/textures/walle-test.png", CollisionShape(glm::vec2(0.0f, 0.0f), 0.05f, false), position, scale, rotation),
            m_speed(speed), rubbishPool(rubbishPool), blockPool(blockPool)
        {
            m_direction = glm::vec2(1.0f, 0.0f);
            camera = SceneManager::getInstance().camera;

            light = std::make_shared<Light>(glm::vec3(0.0f), 0.0f, "#8efcf5");
            SceneManager::getInstance().flashlight = light;

            player = SceneManager::getInstance().soundManager;
            movingSound = nullptr;

            collectionRange = 0.13f;
            m_h_tiles = 3;

            spriteShader->use();
            spriteShader->setVec3("lights[1].color", light->getColor() * light->strength);
            
            processingTimer = std::make_unique<Timer>(2.0f, [this] {
                expellBlock();
                }, false);

            maxScale = scale;

            processingTimer->pause();
        }

        virtual ~Walle() {
            camera.reset();
            light.reset();
            rubbishPool.reset();
            blockPool.reset();

            processingTimer.release();

            if (player != nullptr) {
                player = nullptr;
            }

            if (movingSound != nullptr) {
                movingSound->stop();
                movingSound->drop();
                movingSound = nullptr;
            }
        }


        void processInput(GLFWwindow* window) {
            if (processing) return;

            m_velocity = glm::vec2(0.0f, 0.0f);

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                m_velocity += glm::vec2(0.0f, 1.0f);

            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                m_velocity -= glm::vec2(0.0f, 1.0f);

            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                m_velocity -= glm::vec2(1.0f, 0.0f);

            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                m_velocity += glm::vec2(1.0f, 0.0f);

            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                int picked = 0;
                for (std::shared_ptr<Character> object : SceneManager::getInstance()) {
                    if (std::shared_ptr<Rubbish> rubbish = dynamic_pointer_cast<Rubbish>(object)) {
                        if (rubbish->isPickable && glm::distance(rubbish->getPosition(), m_position + m_direction * collectionRange) <= collectionRange) {
                            rubbish->hide();
                            rubbish->setPosition(glm::vec2(2.0f, 2.0f));
                            picked++;
                            
                            collect(rubbish->trashAmount);

                            rubbishPool->returnToPool(rubbish);
                            SceneManager::getInstance().removeObject(rubbish);
                            break;
                        }
                    }
                }

                StatsManager::getInstance().currentRubbish -= picked;
            }

            if (m_velocity == glm::vec2(0.0f, 0.0f)) {
                if (movingSound != nullptr) {
                    movingSound->stop();
                    movingSound->drop();
                    movingSound = nullptr;
                }
                return;
            }

            if (movingSound == nullptr) {
                movingSound = player->play3D("assets/audio/walle_moving.wav", irrklang::vec3df(m_position.x, m_position.y, 0.0f), true, false, true);
            }
            else {
                movingSound->setPosition(irrklang::vec3df(m_position.x, m_position.y, 0.0f));
            }

            player->setListenerPosition(irrklang::vec3df(m_position.x, m_position.y, 0.0f), irrklang::vec3df(m_direction.x, m_direction.y, 0.0f));

            m_velocity = glm::normalize(m_velocity);
            m_direction = m_velocity;
        }

        void update(float deltaTime) {
            processingTimer->updateTimer(deltaTime);

            if (processing) {
                m_scale.x = abs(sin((float)processingTimer->getElapsed() * 5.0f)) * 0.15f + maxScale.x * 0.8f;
                m_scale.y = (abs(cos((float)processingTimer->getElapsed() * 6.0f)) * 0.15f + maxScale.y * 0.8f) * sign(m_direction.x);
                return;
            }

            m_rotation = glm::degrees(atan2(-m_direction.y, m_direction.x));

            m_scale.y = glm::abs(m_scale.y) * sign(m_direction.x);

            // TODO: capire cosa fare con la velocità
            //m_position += m_velocity * m_speed * deltaTime;
            m_position += m_velocity * (m_speed * 0.316f * sqrt(10.0f - m_collected)) * deltaTime;

            m_position = clamp(glm::vec2(-0.82f, -0.82f) + camera->getPosition2D(), glm::vec2(0.82f, 0.82f) + camera->getPosition2D(), m_position);

            glm::vec2 lightPosition = m_position + m_direction * 0.2f;
            light->position = glm::vec3(lightPosition.x, lightPosition.y, 0.2f);
            light->strength = explerp(light->strength, light_target_strength, deltaTime * 2.0f);
            SceneManager::getInstance().ambientStrength = explerp(SceneManager::getInstance().ambientStrength, ambient_target_strength, deltaTime * 2.0f);

            m_shader->use();
            m_shader->setVec3("ambient", hex_color("#a1d8e8") * SceneManager::getInstance().ambientStrength);
            m_shader->setVec3("lights[1].position", light->position);
            m_shader->setVec3("lights[1].color", light->getColor() * light->strength);
        }

        void collect(int trash) {
            if (movingSound != nullptr) {
                movingSound->stop();
                movingSound->drop();
                movingSound = nullptr;
            }

            m_collected += trash;

            if (m_collected < m_max_rubbish) {
                player->play3D("assets/audio/junk_picked_up.wav", irrklang::vec3df(m_position.x, m_position.y, 0.0f), false);
                return;
            }

            player->play3D("assets/audio/walle_compacting.wav", irrklang::vec3df(m_position.x, m_position.y, 0.0f), false);

            processing = true;
            processingTimer->resume();
            processingTimer->reset();

            m_collected = 0;
        }

        int getCollected() {
            return m_collected;
        }

        void setFlashlight(bool state) {
            light_target_strength = state ? 0.75f : 0.0f;
            ambient_target_strength = state ? 0.06f : 0.3f;
        }

        void expellBlock() {
            processing = false;
            m_scale = maxScale;

            std::shared_ptr<Block> block = blockPool->getInstance();

            block->show();
            block->isPickable = true;
            block->setPosition(m_position + m_direction * 0.2f);
            block->setRubbishPool(rubbishPool);
            block->setBlockPool(blockPool);
            block->setSplittable(true);

            SceneManager::getInstance().addObject(block);
        }

    private:
        glm::vec2 m_direction;
        glm::vec2 m_velocity;

        glm::vec2 maxScale;

        float m_speed;
        float collectionRange;

        int m_collected = 0;
        const int m_max_rubbish = JUNK_TO_BLOCK;

        std::shared_ptr<Camera> camera;
        std::shared_ptr<Light> light;
        std::shared_ptr<ObjectPool<Rubbish>> rubbishPool;
        std::shared_ptr<ObjectPool<Block>> blockPool;
        float light_target_strength;
        float ambient_target_strength;

        std::unique_ptr<Timer> processingTimer;
        bool processing = false;

        irrklang::ISoundEngine* player;
        irrklang::ISound* movingSound;

};

#endif