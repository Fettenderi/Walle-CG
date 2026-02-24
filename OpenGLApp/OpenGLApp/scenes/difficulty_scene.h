#pragma once

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
#include "../core/model.h"

#include "../characters/character.h"
#include "../characters/walle.h"
#include "../characters/eve.h"
#include "../characters/rubbish.h"
#include "../characters/block.h"
#include "../characters/image.h"

#include "../utils.h"

#include "../core/scene.h"

#include "i_menu_subscene.h"

#define INSTANT_BOOT

class DifficultyScene : public Scene, public IMenuSubscene, public IGrabUI {
private:
	std::shared_ptr<Image> backButton;
	std::shared_ptr<Image> easyButton;
	std::shared_ptr<Image> mediumButton;
	std::shared_ptr<Image> hardButton;
	std::shared_ptr<Image> customButton;

	bool isFirstFrame = true;
	bool isFirstLoop = true;

	glm::vec2 previousMousePos = glm::vec2(0.0f);
	glm::vec2 spinVelocity = glm::vec2(0.0f);

	std::unique_ptr<Timer> transitionInTimer;
	std::unique_ptr<Timer> transitionOutTimer;

	SceneManager::SceneID nextScene;

	float debug = 0.0f;
public:
	DifficultyScene(GLFWwindow* windowRef) : Scene(windowRef) {};

	virtual void init() {
		scaleImage = true;

		// time initialization
		lastElapsed = glfwGetTime() - offset;
		elapsed = glfwGetTime() - offset;

		// characters
		backButton = std::make_shared<Image>(spriteShader, "assets/textures/ui/back_button.png", glm::vec2(0.0f, -0.7f), glm::vec2(0.4f, 0.2f));
		
		easyButton = std::make_shared<Image>(spriteShader, "assets/textures/ui/easy.png", glm::vec2(-0.54f, 0.54f), glm::vec2(0.4f, 0.2f), 2.3f);
		mediumButton = std::make_shared<Image>(spriteShader, "assets/textures/ui/medium.png", glm::vec2(0.56f, 0.177f), glm::vec2(0.4f, 0.2f), 2.3f);
		hardButton = std::make_shared<Image>(spriteShader, "assets/textures/ui/hard.png", glm::vec2(-0.54f, -0.186f), glm::vec2(0.4f, 0.2f), 2.3f);
		customButton = std::make_shared<Image>(spriteShader, "assets/textures/ui/custom.png", glm::vec2(0.69f, 0.767f), glm::vec2(0.4f, 0.2f), 1.6f);

		// timer
		transitionInTimer = make_unique<Timer>(2.0f, []() {}, false);

		transitionOutTimer = make_unique<Timer>(2.0f, [this]() {
			transitionTimeout();
			}, false);

		transitionOutTimer->pause();

		//3d models
		SceneManager::getInstance().addObject(backButton);
		SceneManager::getInstance().addObject(easyButton);
		SceneManager::getInstance().addObject(mediumButton);
		SceneManager::getInstance().addObject(hardButton);
		SceneManager::getInstance().addObject(customButton);
	}

	virtual float update() {
		if (inLimbo) return 0.0f;

		// deltaTime calculation
		elapsed = glfwGetTime() - offset;
		deltaTime = elapsed - lastElapsed;
		lastElapsed = elapsed;

		// debug
		changeDebugParameters();

		// menu juice
		buttonUpdate();
		updateGrabUI(window, deltaTime);

		// transitioning
		if (isFirstLoop) {
			isFirstLoop = false;
			transitionInTimer->resume();
		}

		transitionInTimer->updateTimer(deltaTime);
		transitionOutTimer->updateTimer(deltaTime);

		return (float)deltaTime;
	}

	virtual void guiUpdate() {

	}

	virtual void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			double xpos, ypos;
			int width, height;

			glfwGetCursorPos(window, &xpos, &ypos);
			glfwGetWindowSize(window, &width, &height);

			float scX = (float)xpos / (float)width * 2.0f - 1.0f;
			float scY = -(float)ypos / (float)height * 2.0f + 1.0f;

			if (backButton->isMouseOver(glm::vec2(scX, scY))) {
				soundPlayer->play2D("assets/audio/ui_click.wav", false);

				// change subscene
				transitionOutTimer->resume();
				nextScene = SceneManager::SceneID::MMWelcomeScene;
			}

			if (customButton->isMouseOver(glm::vec2(scX, scY))) {
				soundPlayer->play2D("assets/audio/ui_click.wav", false);

				FileManager::getInstance().configPath = "saves/custom_config.json";
				FileManager::getInstance().load(FileManager::CONFIG);

				// change subscene
				transitionOutTimer->resume();
				nextScene = SceneManager::SceneID::MMCustomScene;
			}


			if (easyButton->isMouseOver(glm::vec2(scX, scY))) {
				soundPlayer->play2D("assets/audio/ui_click.wav", false);

				FileManager::getInstance().configPath = "saves/easy_config.json";
				FileManager::getInstance().load(FileManager::CONFIG);
				SceneManager::getInstance().changeScene(SceneManager::SceneID::GameScene, window);
				return;
			}

			if (mediumButton->isMouseOver(glm::vec2(scX, scY))) {
				soundPlayer->play2D("assets/audio/ui_click.wav", false);

				FileManager::getInstance().configPath = "saves/medium_config.json";
				FileManager::getInstance().load(FileManager::CONFIG);

				SceneManager::getInstance().changeScene(SceneManager::SceneID::GameScene, window);
				return;
			}

			if (hardButton->isMouseOver(glm::vec2(scX, scY))) {
				soundPlayer->play2D("assets/audio/ui_click.wav", false);

				FileManager::getInstance().configPath = "saves/hard_config.json";
				FileManager::getInstance().load(FileManager::CONFIG);

				SceneManager::getInstance().changeScene(SceneManager::SceneID::GameScene, window);
				return;
			}
		}
	}

	void stickyMouseCallback(GLFWwindow* window, int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			double xpos, ypos;
			int width, height;

			glfwGetCursorPos(window, &xpos, &ypos);
			glfwGetWindowSize(window, &width, &height);

			glm::vec2 mousePos = glm::vec2((float)xpos / (float)width * 2.0f - 1.0f, -(float)ypos / (float)height * 2.0f + 1.0f);

			if (releaseImage(mousePos, [this]() {
				soundPlayer->play2D("assets/audio/pin_image.wav", false);
				})) return;

			if (backButton->isMouseOver(mousePos)) {
				soundPlayer->play2D("assets/audio/ui_click.wav", false);
			}

			if (customButton->isMouseOver(mousePos)) {
				soundPlayer->play2D("assets/audio/ui_click.wav", false);
				grabImage(mousePos, customButton);
			}

			if (easyButton->isMouseOver(mousePos)) {
				soundPlayer->play2D("assets/audio/ui_click.wav", false);
				grabImage(mousePos, easyButton);
			}

			if (mediumButton->isMouseOver(mousePos)) {
				soundPlayer->play2D("assets/audio/ui_click.wav", false);
				grabImage(mousePos, mediumButton);
			}

			if (hardButton->isMouseOver(mousePos)) {
				soundPlayer->play2D("assets/audio/ui_click.wav", false);
				grabImage(mousePos, hardButton);
			}
		}
	}

	void changeDebugParameters() {
		float velocity = 0.0f;

		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			velocity -= 1.0f;

		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			velocity += 1.0f;

		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			velocity *= 2.0f;

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			velocity /= 4.0f;


		if (velocity == 0.0f) return;

		debug += velocity * (float)deltaTime * 2.0f;
		printf("%f\n", debug);
	}

	void buttonUpdate() {
		// over animation

		double xpos, ypos;
		int width, height;

		glfwGetCursorPos(window, &xpos, &ypos);
		glfwGetWindowSize(window, &width, &height);

		glm::vec2 mousePos = glm::vec2((float)xpos / (float)width * 2.0f - 1.0f, -(float)ypos / (float)height * 2.0f + 1.0f);

		std::function<float(float)> scaleAnim = [](float x) {
			return 1.2f;
			};

		std::function<float(float)> rotStatic = [](float x) {
			return 0.0f;
			};

		backButton->animate(deltaTime, mousePos, [](float x) {
			return sin(x) * 0.05f + 0.95f;
			}, [](float x) {
			return sin(x * 2.0f) * 3.0f;
			}, 3.0f);

		easyButton->animate(deltaTime, mousePos, scaleAnim, rotStatic, 50.0f, 30.0f);

		mediumButton->animate(deltaTime, mousePos, scaleAnim, rotStatic, 50.0f, 30.0f);

		hardButton->animate(deltaTime, mousePos, scaleAnim, rotStatic, 50.0f, 30.0f);

		customButton->animate(deltaTime, mousePos, scaleAnim, rotStatic, 50.0f, 30.0f);

		// transition animation

		float softInOut = (lerp(0.0f, 2.0f, easeInBack(transitionOutTimer->getProgress())) + lerp(2.0f, 0.0f, easeOutBack(transitionInTimer->getProgress())));
		float sharpInOut = (lerp(0.0f, 2.0f, easeInCubic(transitionOutTimer->getProgress())) + lerp(2.0f, 0.0f, easeOutCubic(transitionInTimer->getProgress())));

		backButton->setPosition(glm::vec2(0.0f, -softInOut - 0.7f));

		easyButton->setPosition(glm::vec2(sharpInOut - 0.54f, 0.54f -0.090528f));
		mediumButton->setPosition(glm::vec2(-sharpInOut + 0.56f, 0.177f -0.090528f));
		hardButton->setPosition(glm::vec2(sharpInOut - 0.54f, -0.186f -0.090528f));

		customButton->setPosition(glm::vec2(-sharpInOut + 0.69f, 0.767f));

	}

	void placementUpdate() {
		if (!hasGrabbedImage) return;

		double xpos, ypos;
		int width, height;

		glfwGetCursorPos(window, &xpos, &ypos);
		glfwGetWindowSize(window, &width, &height);

		float scX = (float)xpos / (float)width * 2.0f - 1.0f;
		float scY = -(float)ypos / (float)height * 2.0f + 1.0f;

		grabbedImage->setPosition(glm::vec2(scX, scY));
	}


	void transitionTimeout() {
		SceneManager::getInstance().changeSubscene(SceneManager::getInstance().currentScene, nextScene, window,
			[this](std::shared_ptr<Scene> newSubscene) {
				std::shared_ptr<IMenuSubscene> theSubscene = std::dynamic_pointer_cast<IMenuSubscene>(newSubscene);

				if (theSubscene == nullptr) return;

				theSubscene->spriteShader = spriteShader;
				theSubscene->PBRShader = PBRShader;

				theSubscene->soundPlayer = soundPlayer;
			});
	}

	virtual void end() {
		spriteShader.reset();
		PBRShader.reset();

		backButton.reset();
		easyButton.reset();
		mediumButton.reset();
		hardButton.reset();
		customButton.reset();

		transitionInTimer.release();
		transitionOutTimer.release();

		SceneManager::getInstance().removeAllObjects();
	}
};
