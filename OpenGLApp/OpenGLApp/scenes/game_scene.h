#ifndef GAME_SCENE_H
#define GAME_SCENE_H

#include <memory>

#include <iostream>
#include <string>
#include <format>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <irrKlang.h>

#include "../globals/scene_manager.h"

#include "../core/text.h"
#include "../core/shader.h"
#include "../core/quad.h"
#include "../core/timer.h"
#include "../core/pool.h"
#include "../core/light.h"
#include "../core/camera.h"

#include "../characters/character.h"
#include "../characters/walle.h"
#include "../characters/eve.h"
#include "../characters/wind.h"
#include "../characters/rubbish.h"
#include "../characters/block.h"

#include "../utils.h"

#include "../core/scene.h"

class GameScene : public Scene {
	private:
		std::shared_ptr<Shader> lightedShader;
		std::shared_ptr<Light> sun;

		std::shared_ptr<Wind> wind;
		std::shared_ptr<Mo> mo;
		std::shared_ptr<Eve> eve;
		std::shared_ptr<Walle> walle;

		std::shared_ptr<Camera> camera;
		std::unique_ptr<Text> guiText;

		std::shared_ptr<ObjectPool<Rubbish>> rubbishPool;
		std::shared_ptr<ObjectPool<Block>> blockPool;

		glm::vec3 bgColor = glm::vec3(0.6f, 0.42f, 0.33f);
	public:

		GameScene() = default;

		virtual void init() {
			// Text Provider
			guiText = std::make_unique<Text>("assets/fonts/Antonio/static/Antonio-Bold.ttf");

			// time initialization
			lastElapsed = glfwGetTime();
			elapsed = glfwGetTime();

			// camera
			SceneManager::getInstance().camera = std::make_shared<Camera>();
			camera = SceneManager::getInstance().camera;

			camera->setSpeed(0.4f);
			camera->setMoving(false);

			// shader
			lightedShader = std::make_shared<Shader>("core/shaders/lighted_shader.vs", "core/shaders/lighted_shader.fs");

			// pools initialization
			rubbishPool = std::make_shared<ObjectPool<Rubbish>>(30, lightedShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.4f, 0.32f));
			blockPool = std::make_shared<ObjectPool<Block>>(50, lightedShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.25f, 0.25f));

			// background
			std::shared_ptr<Block> background = std::make_shared<Block>(lightedShader, glm::vec2(0.0f, 2.0f), glm::vec2(10.0f, 10.0f));
			background->isPickable = false;
			background->show();

			// characters
			eve = std::make_shared<Eve>(rubbishPool, lightedShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.28f, 0.4f), 1.0f);
			walle = std::make_shared<Walle>(rubbishPool, blockPool, lightedShader, glm::vec2(0.0f, 0.0f), glm::vec2(0.4f, 0.4f), 0.0f, 1.0f);
			mo = make_shared<Mo>(blockPool, lightedShader, glm::vec2(0.0f, -0.6f), glm::vec2(0.28f, 0.4f), 0.0f, 1.0f);
			wind = make_shared<Wind>(rubbishPool, blockPool, lightedShader, glm::vec2(0.0f, -0.6f));

			SceneManager::getInstance().addObject(background);
			SceneManager::getInstance().addObject(wind);
			SceneManager::getInstance().addObject(mo);
			SceneManager::getInstance().addObject(walle);
			SceneManager::getInstance().addObject(eve);

			// lights setup
			sun = std::make_shared<Light>(glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, "#ffffab");
			SceneManager::getInstance().sun = sun;

			lightedShader->use();
			lightedShader->setInt("mainTexture", 0);

			lightedShader->setVec3("ambientColor", hex_color("#a1d8e8"));
			lightedShader->setVec3("sunColor", sun->getColor());
			lightedShader->setFloat("sunStrength", sun->strength);
		}

		virtual float update() {
			// reset screen
			glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// deltaTime calculation
			elapsed = glfwGetTime();
			deltaTime = elapsed - lastElapsed;
			lastElapsed = elapsed;

			// gui update
			guiText->RenderText(std::format("Collected trash: {}", walle->getCollected()), glm::vec2(0.0f, 0.0f), 1.0f, "#0a1518");

			// global update
			camera->update((float)deltaTime);
			sun->position = glm::vec3(sin(elapsed * 0.1f), 0.0f, cos(elapsed * 0.1f));

			walle->setFlashlight(cos(elapsed * 0.1f) <= 0.0f);

			if (StatsManager::getInstance().collectedBlocks > 20) {
				wind->strength = fmax((float)StatsManager::getInstance().collectedBlocks - 20.0f, 0.0f);
			}

			// shader update
			//for (std::shared_ptr<Shader> shader : shaders) {
			//    shader->use();
			//    shader->setMat4("camera", camera->getViewMatrix());
			//}

			lightedShader->use();
			lightedShader->setMat4("camera", camera->getViewMatrix());
			lightedShader->setVec3("sunPosition", sun->position);
			lightedShader->setVec3("viewPosition", camera->getPosition());

			return (float)deltaTime;
		}


		virtual void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
			if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
				double xpos, ypos;
				int width, height;

				glfwGetCursorPos(window, &xpos, &ypos);
				glfwGetWindowSize(window, &width, &height);

				//coordinate x schermata
				float scX = (float)xpos / (float)width * 2.0f - 1.0f;
				float scY = -(float)ypos / (float)height * 2.0f + 1.0f;

				//destinazione di Mo
				mo->setTarget(glm::vec2(scX, scY) - camera->getPosition2D());
				//printf("MO target: (%f, %f)\n", scX, scY);
			}
		}

		virtual void end() {

		}
};

#endif