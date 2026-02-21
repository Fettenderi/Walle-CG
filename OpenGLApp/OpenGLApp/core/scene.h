#ifndef SCENE_H
#define SCENE_H

#include <memory>

#include <iostream>
#include <string>
#include <format>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <irrKlang.h>

#include "../core/shader.h"
#include "../core/quad.h"
#include "../core/light.h"
#include "../core/camera.h"

#include "../utils.h"

class Scene {
    public:
        std::shared_ptr<Scene> parentScene;
        std::shared_ptr<Scene> currentSubscene;

        Scene(GLFWwindow* windowRef) : window(windowRef), offset(glfwGetTime()) {};

        virtual void init() {
            lastElapsed = glfwGetTime() - offset;
            elapsed = glfwGetTime() - offset;
        }

        virtual float update() {
            // deltaTime calculation
            elapsed = glfwGetTime();
            deltaTime = elapsed - lastElapsed;
            lastElapsed = elapsed;

            return (float)deltaTime;
        }

        virtual void guiUpdate() {
        
        }

        virtual void mouseCallback(GLFWwindow* window, int button, int action, int mods) {

        }

        virtual void windowResizedCallback(GLFWwindow* window, int width, int height) {
        
        }

        virtual void end() {

        }

    protected:
        double deltaTime;
        double lastElapsed;
        double elapsed;
        double offset;
  
        GLFWwindow* window;
};

#endif