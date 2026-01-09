#ifndef IMAGE_H
#define IMAGE_H

#include "../core/shader.h"
#include "../utils.h"

#include "character.h"

class Image : public Character {

public:
    Image(std::shared_ptr<Shader> spriteShader, const char* texturePath, glm::vec2 position, glm::vec2 scale)
        : Character(spriteShader, texturePath, CollisionShape(glm::vec2(0.0f, 0.0f), 0.4f, false), position, scale, 0.0f), maxScale(scale) {
        m_collider.isActive = false;
    };

    void setPosition(glm::vec2 position) {
        m_position = position;
    }

    void setRotation(float rotation) {
        m_rotation = rotation;
    }

    float getRotation() {
        return m_rotation;
    }

    void setScale(float factor) {
        uniformScale = factor;
        m_scale = maxScale * factor;
    }

    float getScale() {
        return uniformScale;
    }

    void setRealScale(glm::vec2 scale) {
        m_scale = scale;
    }

    glm::vec2 getRealScale() {
        return m_scale;
    }

    bool isMouseOver(glm::vec2 mousePos) {
        float halfW = m_scale.x * 0.5f;
        float halfH = m_scale.y * 0.5f;

        return mousePos.x >= m_position.x - halfW &&
            mousePos.x <= m_position.x + halfW &&
            mousePos.y >= m_position.y - halfH &&
            mousePos.y <= m_position.y + halfH;
    }

private:
    glm::vec2 maxScale;
    float uniformScale = 1.0f;

};

#endif