#ifndef COLLISION_SHAPE_H
#define COLLISION_SHAPE_H

#include <glm/glm.hpp>

class CollisionShape {
    public:
        glm::vec2 offset;
        float radius;

        CollisionShape(glm::vec2 offset, float radius) : offset(offset), radius(radius) {}
};
#endif


