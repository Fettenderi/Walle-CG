#ifndef CHARACTER_H
#define CHARACTER_H

#include <memory>
#include <vector>
#include <utility>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <irrKlang.h>
#include "../stb_image.h"

#include "../core/collision_shape.h"
#include "../core/shader.h"
#include "../core/quad.h"

#include "../utils.h"


static unsigned long assignedIds;

class Character {
    protected:
        glm::vec2 m_position;
        glm::vec2 m_scale;
        float m_uniform_scale = 1.0f;
        float m_rotation;
        float m_y_offset = 0.0f;
        bool m_is_visible = true;

        unsigned long id;
        unsigned int m_textureID;
        std::shared_ptr<Shader> m_shader;
        irrklang::ISoundEngine* soundManager;

        CollisionShape m_collider;
        bool solved = false;

        
        int m_h_tiles = 1; //numero colonne atlas
        int m_v_tiles = 1; //numero righe atlas

        //tile attuale
        int m_current_frame_x = 0;
        int m_current_frame_y = 0;

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

            m_position -= vector * magnitude * 0.5f;
        }

        bool intersects(std::shared_ptr<Character> other) {
            if (!other->m_collider.isActive) return false;
            if (!other->m_is_visible) return false;
            if (other.get() == this) return false;
            if (m_collider.isStatic && other->m_collider.isStatic) return false;

            glm::vec2 center = m_position + m_collider.offset;
            glm::vec2 otherCenter = other->m_position + other->m_collider.offset;

            float dist = glm::distance(center, otherCenter);

            return dist <= m_collider.radius * m_uniform_scale + other->m_collider.radius * m_uniform_scale;
        }

        void displaySprite(glm::vec2 pos, glm::vec2 scale, float rot) {
            loadSprite(*m_shader, m_textureID, pos, scale, rot);
        }

    public:

        Character(std::shared_ptr<Shader> spriteShader, const char* texturePath, CollisionShape collider, glm::vec2 position, glm::vec2 scale, const float rotation)
            : m_position(position), m_scale(scale), m_rotation(rotation), m_shader(spriteShader), m_collider(collider) {
            loadTexture(&m_textureID, texturePath, GL_RGBA);
            soundManager = irrklang::createIrrKlangDevice();
            id = assignedIds++;
        }

        virtual ~Character() {
            m_shader.reset();
        }

        virtual void renderSprite() {
            if (!m_is_visible) return;
            loadSprite(*m_shader, m_textureID, m_position, m_scale * m_uniform_scale, m_rotation);
        }

        virtual void processInput(GLFWwindow* window) {}

        virtual void update(float deltaTime) {}

        virtual void hitByBomb() {}

        void collide(std::vector<std::shared_ptr<Character>> objects) {
            if (!m_collider.isActive) return;

            resolve(objects, 0);
        }

        void resetCollisionState() {
            solved = false;
        }

        virtual void clampPosition(glm::vec2 min, glm::vec2 max) {
            m_position = clamp(min, max, m_position);
        }

        glm::vec2 getPosition() const {
            return m_position;
        }

        glm::vec2 getScale() const {
            return m_scale;
        }

        void setUniformScale(float newScale) {
            m_uniform_scale = newScale;
        }

        float getUniformScale() const {
            return m_uniform_scale;
        }

        void setScale(glm::vec2 scale) {
            m_scale = scale;
        }

        void updateTexture(const char* newTexture) {
            loadTexture(&m_textureID, newTexture, GL_RGBA);
        }

        float getY() {
            return m_position.y;

            /*
            glm::mat2 R(
                cos(m_rotation), -sin(m_rotation),
                sin(m_rotation), cos(m_rotation)
            );

            glm::vec2 tl(-m_scale.x, m_scale.y);
            glm::vec2 tr(m_scale.x, m_scale.y);
            glm::vec2 bl(-m_scale.x, -m_scale.y);
            glm::vec2 br(m_scale.x, -m_scale.y);

            tl = R * tl;
            tr = R * tr;
            bl = R * bl;
            br = R * br;


            return m_position.y + std::min({ tl.y, tr.y, bl.y, br.y });*/
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

        //per atlas
        //configura dimensioni griglia
        void setAtlasGrid(int h_tiles, int v_tiles) {
            m_h_tiles = h_tiles;
            m_v_tiles = v_tiles;
        }

        //cambio tile con x e y
        void setTile(int x, int y) {
            m_current_frame_x = x;
            m_current_frame_y = y;
        }

        //cambio tile usando un singolo indice (da 0 a h_tiles*v_tiles - 1)
        void setTileIndex(int index) {
            m_current_frame_x = index % m_h_tiles;
            m_current_frame_y = index / m_h_tiles;
        }

        void changeYOffset(GLFWwindow* window, float deltaTime) {
            float velocity = 0.0f;

            if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
                velocity += 1.0f;

            if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)
                velocity -= 1.0f;

            if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
                velocity *= 2.0f;

            if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
                velocity /= 2.0f;

            loadSprite(*m_shader, m_textureID, m_position + glm::vec2(0.0f, m_y_offset), m_scale * m_uniform_scale * 0.01f, m_rotation);

            m_y_offset += velocity * (float)deltaTime * 0.001f;
            printf("offset: (%f)\n", m_y_offset);
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

        void loadSprite(Shader shader, unsigned int texture, glm::vec2 position, glm::vec2 scale, float rotation, bool offset = false) {
            shader.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);

            glBindVertexArray(Quad::getVAO());

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(position, 0.0f));
            model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, -1.0f));
            model = glm::scale(model, glm::vec3(scale, 1.0f));

            shader.setMat4("model", model);

            //calcoli per texture atlas
            glm::vec2 tilesConfig((float)m_h_tiles, (float)m_v_tiles);
            glm::vec2 currentTile((float)m_current_frame_x, (float)m_current_frame_y);

            // Passiamo i vec2 allo shader (assicurati che il Fragment Shader sia aggiornato come nell'opzione 2!)
            shader.setVec2("tilesConfig", tilesConfig);
            shader.setVec2("currentTile", currentTile);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
};

#endif