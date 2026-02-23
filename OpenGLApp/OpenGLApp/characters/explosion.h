#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "../core/timer.h"
#include "../core/shader.h"
#include "../utils.h"

#include "../globals/scene_manager.h"

#include "character.h"

class Explosion : public Character {

public:

    Explosion(std::shared_ptr<Shader> spriteShader, glm::vec2 position, glm::vec2 scale)
        : Character(spriteShader, "assets/textures/explosion_atlas.png", CollisionShape(glm::vec2(0.0f, 0.0f), 0.1f, true), position, scale, 0.0f)
    {
        m_is_visible = false;
        m_scale = glm::vec2(scale.x * 1.5f, scale.y * 2.5f);
        setAtlasGrid(6, 3);
        setTile(0, 2);

        explosionTimer = std::make_unique<Timer>(2.0f, [this] {
            endExplosion();
            }, false);

        player = SceneManager::getInstance().soundManager;
        explosionSound = nullptr;

        explosionTimer->pause();
    }

    
    ~Explosion() {
        explosionTimer.release();

        if (explosionSound != nullptr) {
            explosionSound->stop();
            explosionSound->drop();
            explosionSound = nullptr;
        }
    }
    
    void update(float deltaTime) {
        explosionTimer->updateTimer(deltaTime);
        if (m_is_visible) {
            
            currentFrame = static_cast<int>(explosionTimer->getElapsed() / frameStep);
            if (currentFrame > 17) {
                currentFrame = 17;
                hide();
                if (explosionSound != nullptr) {
                    explosionSound->stop();
                    explosionSound->drop();
                    explosionSound = nullptr;
                }
            }
                
            //printf("codice arriva all'animazione");
            atlasPosition.x = currentFrame % 6;
            atlasPosition.y = 2 - currentFrame / 6;
            setTile(atlasPosition.x, atlasPosition.y);
        }
    }

    void startExplosion(glm::vec2 position) {
        show();
        m_position = position;
        
        
        if (explosionSound == nullptr) {
            explosionSound = player->play3D("assets/audio/explosion.wav", irrklang::vec3df(m_position.x, m_position.y, 0.0f), true, false, true);
        }

        explosionTimer->resume();
        explosionTimer->reset();
    }

    void endExplosion() {
        //printf("\n\n\nESPLOSIONE FINITA");
        hide();
        if (explosionSound != nullptr) {
            explosionSound->stop();
            explosionSound->drop();
            explosionSound = nullptr;
        }
    }
private:

    float frameStep = 0.11f;
    glm::vec2 atlasPosition;
    int currentFrame = 0;

    glm::vec2 explosionPosition;
    std::unique_ptr<Timer> explosionTimer;

    irrklang::ISoundEngine* player;
    irrklang::ISound* explosionSound;

    
};

#endif