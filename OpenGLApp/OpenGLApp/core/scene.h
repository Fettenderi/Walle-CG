#ifndef SCENE_H
#define SCENE_H

class Scene {
    public:
        Scene() = default;

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

        virtual void mouseCallback(GLFWwindow* window, int button, int action, int mods) {

        }

        virtual void end() {

        }

    protected:
        double deltaTime;
        double lastElapsed;
        double elapsed;
};

#endif