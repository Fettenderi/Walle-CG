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

class InstructionsScene : public Scene, public IMenuSubscene {
private:
	std::shared_ptr<Image> backButton;

	bool isFirstFrame = true;
	bool isFirstLoop = true;

	std::unique_ptr<Timer> transitionInTimer;
	std::unique_ptr<Timer> transitionOutTimer;

	SceneManager::SceneID nextScene;

	float debug = 0.0f;
public:
	InstructionsScene(GLFWwindow* windowRef) : Scene(windowRef) {};

	virtual void init() {
		// time initialization
		lastElapsed = glfwGetTime() - offset;
		elapsed = glfwGetTime() - offset;

		// characters
		backButton = std::make_shared<Image>(spriteShader, "assets/textures/ui/back_button.png", glm::vec2(0.0f, -0.7f + 0.127f), glm::vec2(0.4f, 0.2f));

		// timer
		transitionInTimer = make_unique<Timer>(2.0f, []() {}, false);

		transitionOutTimer = make_unique<Timer>(2.0f, [this]() {
			transitionTimeout();
			}, false);

		transitionOutTimer->pause();

		//3d models
		SceneManager::getInstance().addObject(backButton);
	}

	virtual float update() {
		if (inLimbo) return 0.0f;

		// deltaTime calculation
		elapsed = glfwGetTime() - offset;
		deltaTime = elapsed - lastElapsed;
		lastElapsed = elapsed;

		// menu juice
		buttonUpdate();

		// debug
		changeDebugParameters();

		// change easing parameters
		float softInOut = (lerp(0.0f, 2.0f, easeInBack(transitionOutTimer->getProgress())) + lerp(2.0f, 0.0f, easeOutBack(transitionInTimer->getProgress())));
		float sharpInOut = (lerp(0.0f, 2.0f, easeInCubic(transitionOutTimer->getProgress())) + lerp(2.0f, 0.0f, easeOutCubic(transitionInTimer->getProgress())));

		backButton->setPosition(glm::vec2(0.0f, -softInOut - 0.7f)); //-

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
		printf("debug: (%f)\n", debug);
	}

	void buttonUpdate() {
		double xpos, ypos;
		int width, height;

		glfwGetCursorPos(window, &xpos, &ypos);
		glfwGetWindowSize(window, &width, &height);

		glm::vec2 mousePos = glm::vec2((float)xpos / (float)width * 2.0f - 1.0f, -(float)ypos / (float)height * 2.0f + 1.0f);

		backButton->animate(deltaTime, mousePos, [](float x) {
			return sin(x) * 0.05f + 0.95f;
			}, [](float x) {
				return sin(x * 2.0f) * 3.0f;
				}, 3.0f);
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

		transitionInTimer.release();
		transitionOutTimer.release();

		SceneManager::getInstance().removeAllObjects();
	}
};
