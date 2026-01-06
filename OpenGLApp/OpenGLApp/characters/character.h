#ifndef CHARACTER_H
#define CHARACTER_H

#include <memory>
#include <vector>
#include <utility>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../stb_image.h"

#include "../globals/scene_manager.h"

#include "../core/collision_shape.h"
#include "../core/shader.h"
#include "../core/quad.h"

#include <irrKlang.h>

static unsigned long assignedIds;

class Character {
    protected:
        glm::vec2 m_position;
        glm::vec2 m_scale;
        float m_rotation;
        bool m_is_visible = true;

        unsigned long id;
        unsigned int m_textureID;
        std::shared_ptr<Shader> m_shader;
        irrklang::ISoundEngine* soundManager;

        CollisionShape m_collider;
        bool solved = false;


        void resolve(std::vector<std::shared_ptr<Character>> objects, int starting) {
            if (!m_is_visible || solved) return;

            for (int i = starting; i < objects.size(); i++) {
                if (intersects(objects[i])) {

                    kick(*objects[i].get());
                    objects[i]->kick(*this);

                    solved = true;

                    if (i < objects.size() - 1) {
                        objects[i]->resolve(objects, i + 1);
                    }
                }
            }

            solved = true;
        }

        void kick(Character other) {
            if (m_collider.isStatic) return;

            glm::vec2 center = m_position + m_collider.offset;
            glm::vec2 otherCenter = other.m_position + other.m_collider.offset;

            glm::vec2 vector = glm::normalize(otherCenter - center);
            float magnitude = m_collider.radius + other.m_collider.radius - glm::distance(center, otherCenter);

            m_position -= vector * magnitude;
        }

        bool intersects(std::shared_ptr<Character> other) {
            if (!other->m_is_visible) return false;
            if (other.get() == this) return false;
            if (m_collider.isStatic && other->m_collider.isStatic) return false;

            glm::vec2 center = m_position + m_collider.offset;
            glm::vec2 otherCenter = other->m_position + other->m_collider.offset;

            float dist = glm::distance(center, otherCenter);

            return dist <= m_collider.radius + other->m_collider.radius;
        }

    public:

        Character(std::shared_ptr<Shader> spriteShader, const char* texturePath, CollisionShape collider, glm::vec2 position, glm::vec2 scale, const float rotation)
            : m_position(position), m_scale(scale), m_rotation(rotation), m_shader(spriteShader), m_collider(collider) {
            loadTexture(&m_textureID, texturePath, GL_RGBA);
            soundManager = irrklang::createIrrKlangDevice();
            id = assignedIds++;
        }

        void renderSprite() {
            if (!m_is_visible) return;
            loadSprite(*m_shader, m_textureID, m_position, m_scale, m_rotation);
        }

        virtual void processInput(GLFWwindow* window) {}

        virtual void update(float deltaTime) {}

        void collide(std::vector<std::shared_ptr<Character>> objects) {
            resolve(objects, 0);
        }

        void resetCollisionState() {
            solved = false;
        }

        glm::vec2 getPosition() const {
            return m_position;
        }

        float getY() {
            return m_position.y;
        }

        void hide() {
            m_is_visible = false;
        }

        void show() {
            m_is_visible = true;
        }

        bool operator==(const Character& other) {
            return id == other.id;
        }


    private:
        void loadTexture(unsigned int* texture, const char* textureSource, GLint colorEncoding) {
            glGenTextures(1, texture);
            glBindTexture(GL_TEXTURE_2D, *texture);
            // set the texture wrapping parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            // set texture filtering parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // load image, create texture and generate mipmaps
            int width, height, nrChannels;
            stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis. 
            unsigned char* data = stbi_load(textureSource, &width, &height, &nrChannels, STBI_rgb_alpha);
            if (data) {
                glTexImage2D(GL_TEXTURE_2D, 0, colorEncoding, width, height, 0, colorEncoding, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else {
                std::cout << "Failed to load texture" << std::endl;
            }
            stbi_image_free(data);
        }

        void loadSprite(Shader shader, unsigned int texture, glm::vec2 position, glm::vec2 scale, float rotation) {
            shader.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);

            glBindVertexArray(Quad::getVAO());

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(position, 0.0f));
            model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, -1.0f));
            model = glm::scale(model, glm::vec3(scale, 1.0f));

            shader.setMat4("model", model);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
};

#endif