#ifndef LIGHT_H
#define LIGHT_H

#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "../utils.h"

class Light {
    private:
        glm::vec3 m_color;

    public:
        glm::vec3 position;
        float strength;

        Light(glm::vec3 position, float strength, const std::string& color)
            : position(position), strength(strength), m_color(hex_color(color)) {
        }
        
        glm::vec3 getColor() {
            return m_color;
        }
};

#endif