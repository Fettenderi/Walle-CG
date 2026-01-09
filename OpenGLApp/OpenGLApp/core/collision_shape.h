#ifndef COLLISION_SHAPE_H
#define COLLISION_SHAPE_H

#include <glm/glm.hpp>

class CollisionShape {
    public:
        glm::vec2 offset;
        float radius;
        bool isStatic;
        bool isActive = true;

        CollisionShape(glm::vec2 offset, float radius, bool isStatic) : offset(offset), radius(radius), isStatic(isStatic) {}
};
#endif


