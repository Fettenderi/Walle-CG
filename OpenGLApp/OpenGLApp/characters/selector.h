#pragma once

#include <functional>

#include "../core/shader.h"
#include "../core/text.h"

#include "../utils.h"

#include "../globals/file_manager.h"

#include "character.h"
#include "image.h"

class Selector : public Character {

public:
    std::string description;


    float elapsedOver = 0.0f;

    Selector(std::shared_ptr<Text> text, float size, std::shared_ptr<Shader> spriteShader, const char* texturePath, glm::vec2 position, glm::vec2 scale, float uniformScale = 1.0f)
        : Character(spriteShader, texturePath, CollisionShape(glm::vec2(0.0f, 0.0f), 0.4f, false), position, scale, 0.0f), maxScale(scale) {
        m_collider.isActive = false;
        m_uniform_scale = uniformScale;

        textHandler = text;
        fontSize = size;

        lButton = std::make_shared<Image>(spriteShader, texturePath, position, scale, uniformScale);

        rButton = std::make_shared<Image>(spriteShader, texturePath, position, scale, uniformScale);

        selectedValuePosition = glm::vec2(0.0f, 0.0f);
        descriptionPosition = glm::vec2(0.0f, 0.0f);

        value = "1";
        description = "";
    };

    void renderSprite() {
        if (!m_is_visible) return;
        lButton->setRotation(180);

        lButton->renderSprite();
        rButton->renderSprite();

        textHandler->RenderText(description, toScreenSpace(m_position + descriptionPosition), fontSize * uniformScale, "#000000");
        textHandler->RenderText(value, toScreenSpace(m_position + selectedValuePosition), fontSize * uniformScale, "#000000");
    
        //printf("%f, %f\n", toScreenSpace(m_position + descriptionPosition).x, toScreenSpace(m_position + descriptionPosition).y);
    }

    bool isMouseOver(glm::vec2 mousePos) {
        return lButton->isMouseOver(mousePos) || rButton->isMouseOver(mousePos);
    }
    
    void animate(float deltaTime, glm::vec2 mousePos, std::function<float(float)> scaleAnim, std::function<float(float)> rotAnim, float overAnimSpeed, float idleAnimSpeed = 1.0f) {
        lButton->animate(deltaTime, mousePos, scaleAnim, rotAnim, overAnimSpeed, idleAnimSpeed);
        rButton->animate(deltaTime, mousePos, scaleAnim, rotAnim, overAnimSpeed, idleAnimSpeed);
    }

    void setup(float scale, float distance, float font, std::string desc, glm::vec2 descPos, glm::vec2 valuePos) {
        m_uniform_scale = scale;
        buttonsDistance = distance;
        descriptionPosition = descPos;
        selectedValuePosition = valuePos;
        description = desc;

        fontSize = font;

        lButton->setUniformScale(scale);
        rButton->setUniformScale(scale);

        lButton->setPosition(m_position + glm::vec2(-buttonsDistance, 0.0f));
        rButton->setPosition(m_position + glm::vec2(buttonsDistance, 0.0f));
    }

    void setUniformScale(float newScale) {
        m_uniform_scale = newScale;

        lButton->setUniformScale(newScale);
        rButton->setUniformScale(newScale);
    }

    void setPosition(glm::vec2 newPosition) {
        m_position = newPosition;

        lButton->setPosition(m_position + glm::vec2(-buttonsDistance, 0.0f));
        rButton->setPosition(m_position + glm::vec2(buttonsDistance, 0.0f));
    }

    void setDescPosition(glm::vec2 newPosition) {
        descriptionPosition = newPosition;
    }

    void setValuePosition(glm::vec2 newPosition) {
        selectedValuePosition = newPosition;
    }

    void setDistance(float newDistance) {
        buttonsDistance = newDistance;

        lButton->setPosition(m_position + glm::vec2(-buttonsDistance, 0.0f));
        rButton->setPosition(m_position + glm::vec2(buttonsDistance, 0.0f));
    }

    void updateScreenSize(glm::vec2 newScreenSize) {
        screenSize = newScreenSize;
    }

    void load(std::string loadedValue) {
        value = loadedValue;
    }

    void save(std::string key, std::string secondaryKey = "") {
        FileManager::getInstance().set(FileManager::CONFIG, key, value);

        if (secondaryKey == "")
            FileManager::getInstance().set(FileManager::CONFIG, secondaryKey, value);
    }

    void onClick(glm::vec2 mousePos, std::function<std::string(std::string)> leftClick, std::function<std::string(std::string)> rightClick) {
        if (lButton->isMouseOver(mousePos)) {
            value = leftClick(value);
            return;
        }

        if (rButton->isMouseOver(mousePos)) {
            value = rightClick(value);
            return;
        }
    }


private:
    glm::vec2 maxScale;
    float uniformScale = 1.0f;

    std::shared_ptr<Text> textHandler;
    float fontSize = 1.0f;

    std::shared_ptr<Image> lButton;
    std::shared_ptr<Image> rButton;
    float buttonsDistance = 0.0f;

    glm::vec2 selectedValuePosition;
    glm::vec2 descriptionPosition;

    std::string value;

    glm::vec2 screenSize;

    glm::vec2 toScreenSpace(glm::vec2 position) {
        //printf("%f, %f, %f, %f", position.x, (0.5f * position.x + 0.5f), screenSize.x, (0.5f * position.x + 0.5f) * screenSize.x);

        float x = (0.5f * position.x + 0.5f) * screenSize.x;
        float y = (0.5f * position.y + 0.5f) * screenSize.y;
        return glm::vec2(x, y);
    }
};
