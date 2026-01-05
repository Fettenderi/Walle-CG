#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>

#include "../utils.h"

class Camera {
    private:
        glm::vec2 position;
        float targetY;

        float speed;
        float reachedThreshold = 0.01f;

        bool canMove;
        bool hasTarget = false;

    public:
        Camera() {
            position = glm::vec2(0.0f, 0.0f);
            speed = 0.0f;
            canMove = false;
        }

        void update(float deltaTime) {
            if (!hasTarget) return;

            float dist = abs(position.y - targetY);

            if (dist < reachedThreshold) {
                hasTarget = false;
            }
            else {
                position.y = explerp(position.y, targetY, deltaTime * speed);
            }

            //position += glm::vec2(0.0f, -deltaTime * speed);
        }

        bool isMoving() {
            return canMove;
        }

        glm::vec3 getPosition() {
            return glm::vec3(position, 1.0f);
        }

        glm::vec2 getPosition2D() {
            return position;
        }

        float getY() {
            return position.y;
        }

        glm::mat4 getViewMatrix() {
            return glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));
        }

        void setMoving(bool value) {
            canMove = value;
        }

        void setSpeed(float speed) {
            this->speed = speed;
        }

        void setTarget(float y) {
            targetY = y;
            hasTarget = true;
        }
}; 

#endif