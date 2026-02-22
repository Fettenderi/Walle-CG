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
            
            float temp = to_float(FileManager::getInstance().get(FileManager::CONFIG, "walle_compression_time"));
            if (temp == 0.0f) {
                temp = 2.0f;
                FileManager::getInstance().set(FileManager::CONFIG, "walle_compression_time", std::to_string(temp));
            }

            processingTimer = std::make_unique<Timer>(2.0f, [this] {
                expellBlock();
                }, false);

            temp = to_float(FileManager::getInstance().get(FileManager::CONFIG, "walle_magnet_time"));
            if (temp == 0.0f) {
                temp = 5.0f;
                FileManager::getInstance().set(FileManager::CONFIG, "walle_magnet_time", std::to_string(temp));
            }

            magnetTimer = std::make_unique<Timer>(temp, [this] {
                endMagnetEffect();
                }, false);

            temp = to_float(FileManager::getInstance().get(FileManager::CONFIG, "walle_compressor_boost_time"));
            if (temp == 0.0f) {
                temp = 15.0f;
                FileManager::getInstance().set(FileManager::CONFIG, "walle_compressor_boost_time", std::to_string(temp));
            }

            compressorTimer = std::make_unique<Timer>(temp, [this] {
                endCompressorEffect();
                }, false);

            temp = to_float(FileManager::getInstance().get(FileManager::CONFIG, "walle_confused_time"));
            if (temp == 0.0f) {
                temp = 10.0f;
                FileManager::getInstance().set(FileManager::CONFIG, "walle_confused_time", std::to_string(temp));
            }

            fireExtTimer = std::make_unique<Timer>(temp, [this] {
                endFireExtEffect();
                }, false);

            temp = to_float(FileManager::getInstance().get(FileManager::CONFIG, "walle_stunned_time"));
            if (temp == 0.0f) {
                temp = 3.0f;
                FileManager::getInstance().set(FileManager::CONFIG, "walle_stunned_time", std::to_string(temp));
            }

            bombTimer = std::make_unique<Timer>(temp, [this] {
                endBombEffect();
                }, false);
            
            temp = to_float(FileManager::getInstance().get(FileManager::CONFIG, "walle_charging_time"));
            if (temp == 0.0f) {
                temp = 1.0f;
                FileManager::getInstance().set(FileManager::CONFIG, "walle_charging_time", std::to_string(temp));
            }

            lightBatteryTimer = std::make_unique<Timer>(temp, [this] {
                flashlightBattery = (int)clamp(0.0f, 10.0f, (float)flashlightBattery + (isFlashlightActive ? -1.0f : 1.0f));
                
                if (flashlightBattery == 0) {
                    light_target_strength = 0.1f;
                }

                StatsManager::getInstance().flashlightBattery = flashlightBattery;
                printf("flashlightBattery: %d\n", flashlightBattery);

                }, true);

            maxScale = scale;

            processingTimer->pause();
            magnetTimer->pause();
            compressorTimer->pause();
            fireExtTimer->pause();
        }

        virtual ~Walle() {
            camera.reset();
            light.reset();
            rubbishPool.reset();
            blockPool.reset();

            processingTimer.release();
            lightBatteryTimer.release();

            magnetTimer.release();
            compressorTimer.release();
            fireExtTimer.release();
            bombTimer.release();

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
            if (bombActive) {
                m_velocity = glm::vec2(0.0f, 0.0f);
                return;
            }

            if (processing) return;

            m_velocity = glm::vec2(0.0f, 0.0f);

            if (!fireExtActive) {
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                    m_velocity += glm::vec2(0.0f, 1.0f);

                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                    m_velocity -= glm::vec2(0.0f, 1.0f);

                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                    m_velocity -= glm::vec2(1.0f, 0.0f);

                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                    m_velocity += glm::vec2(1.0f, 0.0f);
            }
            else {
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                    m_velocity -= glm::vec2(0.0f, 1.0f);

                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                    m_velocity += glm::vec2(0.0f, 1.0f);

                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                    m_velocity += glm::vec2(1.0f, 0.0f);

                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                    m_velocity -= glm::vec2(1.0f, 0.0f);
            }
            

            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                int picked = 0;
                for (std::shared_ptr<Character> object : SceneManager::getInstance()) {
                    if (std::shared_ptr<Rubbish> rubbish = dynamic_pointer_cast<Rubbish>(object)) {
                        if (rubbish->isPickable && glm::distance(rubbish->getPosition(), m_position + m_direction * collectionRange) <= collectionRange) {
                            rubbish->hide();
                            rubbish->setPosition(glm::vec2(2.0f, 2.0f));
                            picked++;
                            
                            rubbishEffectActivate(rubbish->executeEffect());
                            rubbish->setUniformScale(1.0f);

                            collect(rubbish->trashAmount);

                            rubbishPool->returnToPool(rubbish);
                            SceneManager::getInstance().removeObject(rubbish);
                            break;
                        }
                    }
                }

                StatsManager::getInstance().currentRubbish -= picked;
            }

            lightBatteryTimer->resume();

            if (m_velocity == glm::vec2(0.0f, 0.0f)) {                
                if (movingSound != nullptr) {
                    movingSound->stop();
                    movingSound->drop();
                    movingSound = nullptr;
                }
                return;
            }

            if (!isFlashlightActive) {
                lightBatteryTimer->pause();
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
            magnetTimer->updateTimer(deltaTime);
            compressorTimer->updateTimer(deltaTime);
            fireExtTimer->updateTimer(deltaTime);
            bombTimer->updateTimer(deltaTime);
            lightBatteryTimer->updateTimer(deltaTime);

            if (processing) {
                m_scale.x = abs(sin((float)processingTimer->getElapsed() * 5.0f)) * 0.15f + maxScale.x * 0.8f;
                m_scale.y = (abs(cos((float)processingTimer->getElapsed() * 6.0f)) * 0.15f + maxScale.y * 0.8f) * sign(m_direction.x);
                return;
            }

            m_rotation = glm::degrees(atan2(-m_direction.y, m_direction.x));

            m_scale.y = glm::abs(m_scale.y) * sign(m_direction.x);

            // TODO: capire cosa fare con la velocitE
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


            if(magnetActive)
                for (std::shared_ptr<Character> object : SceneManager::getInstance()) {
                    if (std::shared_ptr<Rubbish> rubbish = dynamic_pointer_cast<Rubbish>(object)) {
                        if (glm::distance(rubbish->getPosition(), m_position) <= 0.7) {
                            rubbish->moveToWalle(this->getPosition());
                        }
                    }
                }
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
            if (isFlashlightActive == state) return;

            isFlashlightActive = state;

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


        void rubbishEffectActivate(int effectNumber) {
            switch (effectNumber) {
                case 0:
                    break;
                case 1: //magnete
                    magnetActive = true;
                    magnetTimer->resume();
                    magnetTimer->reset();
                    break;
                case 2: //compressori
                    processingTimer->changeDuration(1.0f);
                    compressorTimer->resume();
                    compressorTimer->reset();
                    break;
                case 3: //estintore
                    fireExtActive = true;
                    fireExtTimer->resume();
                    fireExtTimer->reset();
                    break;
                case 4: 
                    break;
            }
        }

        void endMagnetEffect() {
            magnetActive = false;
        }

        void endCompressorEffect() {
            processingTimer->changeDuration(2.0f);
        }

        void endFireExtEffect() {
            fireExtActive = false;
        }

        void endBombEffect() {
            bombActive = false;
        }

        void hitByBomb() {
            bombActive = true;
            bombTimer->resume();
            bombTimer->reset();
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
        std::shared_ptr<ObjectPool<Rubbish>> rubbishPool;
        std::shared_ptr<ObjectPool<Block>> blockPool;

        std::shared_ptr<Light> light;
        std::unique_ptr<Timer> lightBatteryTimer;
        float light_target_strength = 0.0f;
        float ambient_target_strength = 0.3f;
        bool isFlashlightActive = false;
        int flashlightBattery = 5;

        std::unique_ptr<Timer> processingTimer;
        bool processing = false;

        std::unique_ptr<Timer> magnetTimer;
        std::unique_ptr<Timer> compressorTimer;
        std::unique_ptr<Timer> fireExtTimer;
        std::unique_ptr<Timer> bombTimer;
        bool magnetActive = false;
        bool fireExtActive = false;
        bool bombActive = false;

        irrklang::ISoundEngine* player;
        irrklang::ISound* movingSound;



};

#endif