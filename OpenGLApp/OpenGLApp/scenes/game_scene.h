//#ifndef GAME_SCENE_H
//#define GAME_SCENE_H
//
//#include <memory>
//
//#include <iostream>
//#include <string>
//#include <format>
//
//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
//
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//
//#include <irrKlang.h>
//
//#include "../globals/scene_manager.h"
//
//#include "../core/text.h"
//#include "../core/shader.h"
//#include "../core/quad.h"
//#include "../core/timer.h"
//#include "../core/pool.h"
//#include "../core/light.h"
//#include "../core/camera.h"
//
//#include "../characters/character.h"
//#include "../characters/walle.h"
//#include "../characters/mo.h"
//#include "../characters/eve.h"
//#include "../characters/rubbish.h"
//#include "../characters/block.h"
//
//#include "../utils.h"
//
//#include "../core/scene.h"
//
//class GameScene : public Scene {
//    public:
//        std::unique_ptr<ObjectPool<Rubbish>> rubbishPool;
//        std::unique_ptr<ObjectPool<Block>> blockPool;
//
//        GameScene() = default;
//
//        virtual void init() {
//            // pools initialization
//            rubbishPool = std::make_unique<ObjectPool<Rubbish>>(5, lightedShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.4f, 0.32f));
//            blockPool = std::make_unique<ObjectPool<Block>>(23, lightedShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.25f, 0.25f));
//
//            // background
//            std::shared_ptr<Block> lightedBlock = std::make_shared<Block>(lightedShader, glm::vec2(0.0f, 0.0f), glm::vec2(10.0f, 10.0f));
//            lightedBlock->isPickable = false;
//            lightedBlock->show();
//
//            // characters
//            std::shared_ptr<Walle> walle = std::make_shared<Walle>(lightedShader, glm::vec2(0.0f, 0.0f), glm::vec2(0.4f, 0.4f), 0.0f, 1.0f);
//            std::shared_ptr<Eve> eve = std::make_shared<Eve>(lightedShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.28f, 0.4f), 1.0f);
//            mo = make_shared<Mo>(lightedShader, glm::vec2(0.0f, -0.6f), glm::vec2(0.28f, 0.4f), 0.0f, 1.0f);
//
//            SceneManager::getInstance().addObject(lightedBlock);
//            SceneManager::getInstance().addObject(mo);
//            SceneManager::getInstance().addObject(walle);
//            SceneManager::getInstance().addObject(eve);
//
//            // lights setup
//            std::shared_ptr<Light> sun = make_shared<Light>(glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, "#ffffab");
//            SceneManager::getInstance().sun = sun;
//
//            lightedShader->use();
//            lightedShader->setInt("mainTexture", 0);
//
//            glm::vec3 bgColor = glm::vec3(0.6f, 0.42f, 0.33f);
//
//            lightedShader->setVec3("ambientColor", hex_color("#a1d8e8"));
//            lightedShader->setVec3("sunColor", sun->getColor());
//            lightedShader->setFloat("sunStrength", sun->strength);
//
//        }
//
//        virtual void update(float deltaTime) {
//
//        }
//
//        virtual void end() {
//
//        }
//};
//
//#endif