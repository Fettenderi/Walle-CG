#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <vector>

#include "../utils.h"

class Camera {
    private:
        glm::vec2 position;
        float targetY;

        float speed;
        float reachedThreshold = 0.01f;

        bool canMove;
        bool hasTarget = false;

        //for 3d
        glm::vec3 position3d = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    public:
        Camera() {
            position = glm::vec2(0.0f, 0.0f);
            speed = 0.0f;
            targetY = 0.0f;
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

        float getTargetY() {
            return targetY;
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

        void moveTarget(float offset) {
            targetY += offset;
            hasTarget = true;
        }

        // returns the view matrix calculated using Euler Angles and the LookAt Matrix
        glm::mat4 getViewMatrix3d()
        {
            return glm::lookAt(position3d, position3d + front, up);
        }
}; 

#endif