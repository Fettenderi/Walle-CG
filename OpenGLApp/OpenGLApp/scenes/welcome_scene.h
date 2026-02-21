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

class WelcomeScene : public Scene, public IMenuSubscene {
private:
	std::shared_ptr<Image> mouseDetector;

	std::shared_ptr<Image> walleGuide;
	std::shared_ptr<Image> moGuide;
	std::shared_ptr<Image> difficultyButton;
	std::shared_ptr<Image> instructionsButton;

	std::shared_ptr<Model> walle;
	std::shared_ptr<Model> logo;

	glm::vec3 bgColor = hex_color("#000000");

	std::shared_ptr<Image> grabbedImage;
	bool hasGrabbedImage = false;
	bool isFirstFrame = true;

	glm::vec2 previousMousePos = glm::vec2(0.0f);
	glm::vec2 spinVelocity = glm::vec2(0.0f);

	float modelOffset = 0.0f;
	float logoOffset = 0.0f;

	float debug = 0.0f;
public:
	WelcomeScene(GLFWwindow* windowRef) : Scene(windowRef) {};

	virtual void init() {
		// time initialization
		lastElapsed = glfwGetTime() - offset;
		elapsed = glfwGetTime() - offset;

		// characters
		mouseDetector = std::make_shared<Image>(spriteShader, "assets/textures/bg_menu_placeholder.png", glm::vec2(0.0f, 0.227f), glm::vec2(1.134f, 1.258f));

		walleGuide = std::make_shared<Image>(spriteShader, "assets/textures/instructions_walle.png", glm::vec2(-0.657143f, -0.490476f), glm::vec2(0.5f, 0.8f));
		moGuide = std::make_shared<Image>(spriteShader, "assets/textures/instructions_mo.png", glm::vec2(0.691071f, -0.554762f), glm::vec2(0.5f, 0.8f));
		difficultyButton = std::make_shared<Image>(spriteShader, "assets/textures/play_button.png", glm::vec2(debug, -0.7f), glm::vec2(0.4f, 0.2f));
		instructionsButton = std::make_shared<Image>(spriteShader, "assets/textures/play_button.png", glm::vec2(-debug, -0.7f), glm::vec2(0.4f, 0.2f));

		//3d models
		walle = std::make_shared<Model>("assets/models/walle/walle.gltf");
		logo = std::make_shared<Model>("assets/models/logo/logo.obj");

		SceneManager::getInstance().addObject(walleGuide);
		SceneManager::getInstance().addObject(moGuide);
		SceneManager::getInstance().addObject(difficultyButton);
		SceneManager::getInstance().addObject(instructionsButton);
	}

	virtual float update() {
		// deltaTime calculation
		elapsed = glfwGetTime() - offset;
		deltaTime = elapsed - lastElapsed;
		lastElapsed = elapsed;

		// menu juice
		handleMouseModelInteraction();
		placementUpdate();
		buttonUpdate();

		// rendering the loaded models
		glm::mat4 walleModelMat = glm::mat4(1.0f);
		walleModelMat = glm::translate(walleModelMat, glm::vec3(0.0f, -0.13f, -0.85f));
		walleModelMat = glm::scale(walleModelMat, glm::vec3(0.072f));
		walleModelMat = glm::rotate(walleModelMat, glm::radians((float)elapsed * 10.0f) + modelOffset, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 logoModelMat = glm::mat4(1.0f);
		logoModelMat = glm::translate(logoModelMat, glm::vec3(0.0f, 0.19f, -0.72f));
		logoModelMat = glm::scale(logoModelMat, glm::vec3(0.085f));
		logoModelMat = glm::rotate(logoModelMat, glm::radians((float)sin(elapsed * 2.0f) * 5.0f) + logoOffset, glm::vec3(0.0f, 0.0f, 1.0f));
		logoModelMat = glm::rotate(logoModelMat, glm::radians((float)cos(elapsed * 2.0f) * 5.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glEnable(GL_DEPTH_TEST);

		// models update
		PBRShader->use();
		PBRShader->setMat4("model", walleModelMat);
		walle->Draw(*PBRShader);

		PBRShader->setMat4("model", logoModelMat);
		logo->Draw(*PBRShader);

		glDisable(GL_DEPTH_TEST);

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

			if (hasGrabbedImage) {
				hasGrabbedImage = false;
				grabbedImage->setPosition(glm::vec2(scX, scY));
				grabbedImage = nullptr;
				printf("(%f, %f)", scX, scY);
				soundPlayer->play2D("assets/audio/pin_image.wav", false);
				return;
			}

			if (walleGuide->isMouseOver(glm::vec2(scX, scY))) {
				hasGrabbedImage = true;
				grabbedImage = walleGuide;
				soundPlayer->play2D("assets/audio/unpin_image.wav", false);
			}

			if (moGuide->isMouseOver(glm::vec2(scX, scY))) {
				hasGrabbedImage = true;
				grabbedImage = moGuide;
				soundPlayer->play2D("assets/audio/unpin_image.wav", false);
			}

			if (difficultyButton->isMouseOver(glm::vec2(scX, scY))) {
				soundPlayer->play2D("assets/audio/ui_click.wav", false);

				// change subscene
				SceneManager::getInstance().changeSubscene(SceneManager::getInstance().currentScene, SceneManager::SceneID::MMDifficultyScene, window,
				[this](std::shared_ptr<Scene> newSubscene) {
					std::shared_ptr<IMenuSubscene> theSubscene = std::dynamic_pointer_cast<IMenuSubscene>(newSubscene);

					if (theSubscene == nullptr) return;

					theSubscene->spriteShader = spriteShader;
					theSubscene->PBRShader = PBRShader;

					theSubscene->soundPlayer = soundPlayer;
					});
			}

			if (instructionsButton->isMouseOver(glm::vec2(scX, scY))) {
				soundPlayer->play2D("assets/audio/ui_click.wav", false);

				// change subscene
				SceneManager::getInstance().changeSubscene(SceneManager::getInstance().currentScene, SceneManager::SceneID::MMInstructionsScene, window,
					[this](std::shared_ptr<Scene> newSubscene) {
						std::shared_ptr<IMenuSubscene> theSubscene = std::dynamic_pointer_cast<IMenuSubscene>(newSubscene);

						if (theSubscene == nullptr) return;

						theSubscene->spriteShader = spriteShader;
						theSubscene->PBRShader = PBRShader;

						theSubscene->soundPlayer = soundPlayer;
					});
			}
		}
	}

	void buttonUpdate() {
		double xpos, ypos;
		int width, height;

		glfwGetCursorPos(window, &xpos, &ypos);
		glfwGetWindowSize(window, &width, &height);

		float scX = (float)xpos / (float)width * 2.0f - 1.0f;
		float scY = -(float)ypos / (float)height * 2.0f + 1.0f;


		// codice potrebbe essere spostato in Image
		if (difficultyButton->isMouseOver(glm::vec2(scX, scY))) {
			difficultyButton->elapsedOver += (float)deltaTime;
			difficultyButton->setScale(explerp(difficultyButton->getScale(), sin(difficultyButton->elapsedOver) * 0.05f + 0.95f, (float)deltaTime * 3.0f));
			difficultyButton->setRotation(explerp(difficultyButton->getRotation(), sin(difficultyButton->elapsedOver * 2.0f) * 3.0f, (float)deltaTime * 3.0f));
		}
		else {
			difficultyButton->setScale(explerp(difficultyButton->getScale(), 1.0f, (float)deltaTime * 1.0f));
			difficultyButton->setRotation(explerp(difficultyButton->getRotation(), 0.0f, (float)deltaTime * 1.0f));

			difficultyButton->elapsedOver = explerp(difficultyButton->elapsedOver, 0.0f, (float)deltaTime * 1.0f);
		}

		if (instructionsButton->isMouseOver(glm::vec2(scX, scY))) {
			instructionsButton->elapsedOver += (float)deltaTime;
			instructionsButton->setScale(explerp(instructionsButton->getScale(), sin(instructionsButton->elapsedOver) * 0.05f + 0.95f, (float)deltaTime * 3.0f));
			instructionsButton->setRotation(explerp(instructionsButton->getRotation(), sin(instructionsButton->elapsedOver * 2.0f) * 3.0f, (float)deltaTime * 3.0f));
		}
		else {
			instructionsButton->setScale(explerp(instructionsButton->getScale(), 1.0f, (float)deltaTime * 1.0f));
			instructionsButton->setRotation(explerp(instructionsButton->getRotation(), 0.0f, (float)deltaTime * 1.0f));

			instructionsButton->elapsedOver = explerp(instructionsButton->elapsedOver, 0.0f, (float)deltaTime * 1.0f);
		}
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

	void handleMouseModelInteraction() {
		double xpos, ypos;
		int width, height;

		glfwGetCursorPos(window, &xpos, &ypos);
		glfwGetWindowSize(window, &width, &height);

		float scX = (float)xpos / (float)width * 2.0f - 1.0f;
		float scY = -(float)ypos / (float)height * 2.0f + 1.0f;

		glm::vec2 currentMousePos = glm::vec2(scX, scY);

		glm::vec2 dist = currentMousePos - previousMousePos;

		if (isFirstFrame) {
			isFirstFrame = false;
			previousMousePos = currentMousePos;
			return;
		}

		if (mouseDetector->isMouseOver(glm::vec2(scX, scY)) &&
			((abs(dist.x) > 0.005f) || (abs(dist.y) > 0.005f))) {
			spinVelocity.x += dist.x * 10.0f * (float)deltaTime;
			spinVelocity.y += dist.y * 10.0f * (float)deltaTime * sign(currentMousePos.x);
		}
		else {
			spinVelocity.x = explerp(spinVelocity.x, 0.0f, (float)deltaTime * 1.5f);
			spinVelocity.y = explerp(spinVelocity.y, 0.0f, (float)deltaTime * 1.5f);
		}


		modelOffset += spinVelocity.x * 50.0f * (float)deltaTime;
		logoOffset += spinVelocity.y * 50.0f * (float)deltaTime;

		previousMousePos = currentMousePos;
	}

	virtual void windowResizedCallback(GLFWwindow* window, int width, int height) {
		glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

		PBRShader->use();
		PBRShader->setMat4("projection", perspectiveProjection);

		isFirstFrame = true;
	}

	virtual void end() {
		spriteShader.reset();
		PBRShader.reset();

		mouseDetector.reset();

		walleGuide.reset();
		moGuide.reset();
		difficultyButton.reset();
		instructionsButton.reset();

		walle.reset();
		logo.reset();

		grabbedImage.reset();

		SceneManager::getInstance().removeAllObjects();
	}
};
