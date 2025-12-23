#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>

class Camera {
    private:
        glm::vec2 position;
        float speed;
        Shader* shader;
        bool canMove;

    public:
        Camera() {
            position = glm::vec2(0.0f, 0.0f);
            speed = 0.0f;
            canMove = false;
        }

        void update(float deltaTime) {
            if (!canMove) return;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(position, 0.0f));

            position += glm::vec2(0.0f, -deltaTime * speed);

            shader->setMat4("camera", model);
        }

        bool isMoving() {
            return canMove;
        }

        void setMoving(bool value) {
            canMove = value;
        }

        void setShader(Shader* shader) {
            this->shader = shader;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(position, 0.0f));

            this->shader->setMat4("camera", model);

        }

        void setSpeed(float speed) {
            this->speed = speed;
        }
}; 

#endif