#ifndef SCENE_H
#define SCENE_H

class Scene {
    public:
        Scene(GLFWwindow* windowRef) : window(windowRef) {};

        virtual void init() {
            lastElapsed = glfwGetTime();
            elapsed = glfwGetTime();
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

        virtual void end() {

        }

    protected:
        double deltaTime;
        double lastElapsed;
        double elapsed;
  
        GLFWwindow* window;
};

#endif