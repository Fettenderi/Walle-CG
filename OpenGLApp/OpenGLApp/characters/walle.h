#ifndef WALLE_H
#define WALLE_H

#include "../utils.h"

#include "character.h"
#include "rubbish.h"
#include "block.h"

#include "../globals/scene_manager.h"

#include "../core/light.h"

#include <cmath>

class Walle : public Character {

    public:
        Walle(std::shared_ptr<ObjectPool<Rubbish>> rubbishPool, std::shared_ptr<ObjectPool<Block>> blockPool, std::shared_ptr<Shader> spriteShader, glm::vec2 position, glm::vec2 scale, const float rotation, const float speed)
            : Character(spriteShader, "assets/textures/walle.png", position, scale, rotation), m_speed(speed), rubbishPool(rubbishPool), blockPool(blockPool)
        {
            m_direction = glm::vec2(1.0f, 0.0f);
            camera = SceneManager::getInstance().camera;

            light = std::make_shared<Light>(glm::vec3(0.0f), 0.0f, "#8efcf5");
            SceneManager::getInstance().flashlight = light;

            spriteShader->use();
            spriteShader->setVec3("lightColor", light->getColor());
            spriteShader->setFloat("lightStrength", light->strength);
        }

        void processInput(GLFWwindow* window) {
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
                for (std::shared_ptr<Character> object : SceneManager::getInstance()) {
                    if (std::shared_ptr<Rubbish> rubbish = dynamic_pointer_cast<Rubbish>(object)) {
                        if (rubbish->isPickable && glm::distance(rubbish->getPosition(), m_position) <= m_collection_distance) {
                            rubbish->hide();
                            rubbish->setPosition(glm::vec2(2.0f, 2.0f));

                            rubbishPool->returnToPool(rubbish);
                            SceneManager::getInstance().removeObject(rubbish);

                            collect();
                            break;
                        }
                    }
                }
            }

            if (m_velocity == glm::vec2(0.0f, 0.0f)) return;

            m_velocity = glm::normalize(m_velocity);
            m_direction = m_velocity;
        }

        void update(float deltaTime) {
            m_rotation = glm::degrees(atan2(-m_direction.y, m_direction.x));

            m_scale.y = glm::abs(m_scale.y) * sign(m_direction.x);

            m_position += m_velocity * m_speed * deltaTime;

            m_position = clamp(glm::vec2(-0.82f, -0.82f) - camera->getPosition2D(), glm::vec2(0.82f, 0.82f) - camera->getPosition2D(), m_position);


            glm::vec2 lightPosition = m_position + m_direction * 0.2f;
            light->position = glm::vec3(lightPosition.x, lightPosition.y, 0.2f);
            light->strength = explerp(light->strength, light_target_strength, deltaTime * 3.0f);

            m_shader->use();
            m_shader->setVec3("lightPosition", light->position);
            m_shader->setFloat("lightStrength", light->strength);
        }

        void collect() {
            // soundManager->play2D("assets/audio/bell.wav", false);

            m_collected++;

            if (m_collected < m_max_rubbish) return;

            std::shared_ptr<Block> block = blockPool->getInstance();

            block->show();
            block->isPickable = true;
            block->setPosition(m_position + m_direction * 0.2f);

            SceneManager::getInstance().addObject(block);

            m_collected = 0;
        }

        int getCollected() {
            return m_collected;
        }

        void setFlashlight(bool state) {
            light_target_strength = state ? 0.75f : 0.0f;
        }

        void free() { }


    private:
        glm::vec2 m_direction;
        glm::vec2 m_velocity;

        float m_speed;
        float m_collection_distance = 0.2f;

        int m_collected = 0;
        const int m_max_rubbish = 1;

        std::shared_ptr<Camera> camera;
        std::shared_ptr<Light> light;
        std::shared_ptr<ObjectPool<Rubbish>> rubbishPool;
        std::shared_ptr<ObjectPool<Block>> blockPool;
        float light_target_strength;
};

#endif