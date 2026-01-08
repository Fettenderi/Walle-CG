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

		std::shared_ptr<Image> background;
		std::shared_ptr<Wind> wind;
		std::shared_ptr<Mo> mo;
		std::shared_ptr<Eve> eve;
		std::shared_ptr<Walle> walle;

		std::shared_ptr<Camera> camera;
		std::unique_ptr<Text> guiText;

		std::shared_ptr<ObjectPool<Rubbish>> rubbishPool;
		std::shared_ptr<ObjectPool<Block>> blockPool;

		glm::vec3 bgColor = glm::vec3(0.6f, 0.42f, 0.33f);

		bool lmbPressed = false;
	public:

		GameScene(GLFWwindow* windowRef) : Scene(windowRef) {};

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
			rubbishPool = std::make_shared<ObjectPool<Rubbish>>(10, lightedShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.4f, 0.32f));
			blockPool = std::make_shared<ObjectPool<Block>>(30, lightedShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.25f, 0.25f));

			// background
			background = std::make_shared<Image>(lightedShader, "assets/textures/block.png", glm::vec2(0.0f, 0.0f), glm::vec2(10.0f, 10.0f));

			// characters
			eve = std::make_shared<Eve>(rubbishPool, lightedShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.28f, 0.4f), 1.0f);
			walle = std::make_shared<Walle>(rubbishPool, blockPool, lightedShader, glm::vec2(0.0f, 0.0f), glm::vec2(0.4f, 0.4f), 0.0f, 1.0f);
			mo = make_shared<Mo>(blockPool, lightedShader, glm::vec2(0.0f, -0.6f), glm::vec2(0.28f, 0.4f), 0.0f, 1.0f);
			wind = make_shared<Wind>(rubbishPool, blockPool, lightedShader, glm::vec2(0.0f, -0.6f));

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
			//elapsed = glfwGetTime() * 1.75f;
			elapsed = glfwGetTime();
			deltaTime = elapsed - lastElapsed;
			lastElapsed = elapsed;

			// global update
			camera->update((float)deltaTime);
			background->setPosition(glm::vec2(0.0f, 2.0f) - camera->getPosition2D());
			background->renderSprite();
			sun->position = glm::vec3(sin(elapsed * 0.1f), 0.0f, cos(elapsed * 0.1f));

			walle->setFlashlight(cos(elapsed * 0.1f) <= 0.0f);
			wind->updateStrength((float)StatsManager::getInstance().collectedBlocks);
			if (StatsManager::getInstance().collectedBlocks / BLOCK_COLUMNS > 3) {
				camera->setTarget(explerp(camera->getTargetY(), 0.0f, (float)deltaTime * 0.05f));
			}

			lightedShader->use();
			lightedShader->setMat4("camera", camera->getViewMatrix());
			lightedShader->setVec3("sunPosition", sun->position);
			lightedShader->setVec3("viewPosition", camera->getPosition());

			// mo movement
			if (lmbPressed) {
				double xpos, ypos;
				int width, height;

				glfwGetCursorPos(window, &xpos, &ypos);
				glfwGetWindowSize(window, &width, &height);

				//coordinate x schermata
				float scX = (float)xpos / (float)width * 2.0f - 1.0f;
				float scY = -(float)ypos / (float)height * 2.0f + 1.0f;

				//destinazione di Mo
				mo->setTarget(glm::vec2(scX, scY) - camera->getPosition2D());
			}

			return (float)deltaTime;
		}

		virtual void guiUpdate() {
			int width, height;
			glfwGetWindowSize(window, &width, &height);
			
			guiText->RenderText(std::format("{:02.0f}:{:02.0f}", floor((float)elapsed / 60.0f), mod((float)elapsed, 60.0f)), glm::vec2(width / 2.0f - 34.0f, height - 40.0f), 0.7f, "#0a1518");
			guiText->RenderText(std::format("Blocks: {}", StatsManager::getInstance().collectedBlocks), glm::vec2(10.0f, 50.0f), 0.7f, "#0a1518");
			guiText->RenderText(std::format("Strength: {:.2f}", wind->getStrength()), glm::vec2(10.0f, 90.0f), 0.7f, "#0a1518");

			glm::vec2 camPosition = SceneManager::getInstance().camera->getPosition2D();
			float pileHeight = StatsManager::getInstance().maxBlockProgress;
			
			glm::vec2 min = glm::vec2(-0.8f + camPosition.x, remap(-0.8f, -1.0f, 1.0f, fmax(-1.0f - camPosition.y, pileHeight), 1.0f - camPosition.y));
			glm::vec2 max = glm::vec2(0.8f + camPosition.x, remap(0.8f, -1.0f, 1.0f, fmax(-1.0f - camPosition.y, pileHeight), 1.0f - camPosition.y));
			guiText->RenderText(std::format("Min: ({:.2f}, {:.2f}), Max: ({:.2f}, {:.2f})", min.x, min.y, max.x, max.y), glm::vec2(10.0f, 10.0f), 0.7f, "#0a1518");

		}


		virtual void mouseCallback(GLFWwindow* passedWindow, int button, int action, int mods) {
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				lmbPressed = action == GLFW_PRESS;
			}
		}

		virtual void end() {

		}
};

#endif