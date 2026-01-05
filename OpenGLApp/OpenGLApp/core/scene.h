#ifndef SCENE_H
#define SCENE_H

class Scene {
    public:
        Scene() = default;

        virtual void init();
        virtual void update(float deltaTime);
        virtual void end();
};

#endif