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
#include "../characters/selector.h"

#include "../utils.h"

#include "../core/scene.h"

#include "i_menu_subscene.h"
#include "i_grab_ui.h"

#define INSTANT_BOOT

class CustomScene : public Scene, public IMenuSubscene {
private:
	std::shared_ptr<Selector> paramSelectors[6];
	glm::vec2 selectorPositions[6];

	std::shared_ptr<Image> playButton;
	std::shared_ptr<Image> backButton;

	std::shared_ptr<Text> textHandler;

	bool isFirstFrame = true;
	bool isFirstLoop = true;

	std::unique_ptr<Timer> transitionInTimer;
	std::unique_ptr<Timer> transitionOutTimer;

	SceneManager::SceneID nextScene;

	glm::vec2 debug = glm::vec2(0.0f, 0.0f);
public:
	CustomScene(GLFWwindow* windowRef) : Scene(windowRef) {};

	virtual void init() {
		// time initialization
		lastElapsed = glfwGetTime() - offset;
		elapsed = glfwGetTime() - offset;

		// text handler
		textHandler = std::make_shared<Text>("assets/fonts/Antonio/static/Antonio-Bold.ttf");

		glm::ivec2 screenSizeI;
		glfwGetWindowSize(window, &(screenSizeI.x), &(screenSizeI.y));

		// characters
		backButton = std::make_shared<Image>(spriteShader, "assets/textures/ui/back_button.png", glm::vec2(0.0f, -0.9f), glm::vec2(0.4f, 0.2f));
		playButton = std::make_shared<Image>(spriteShader, "assets/textures/ui/play_button.png", glm::vec2(0.0f, -0.9f), glm::vec2(0.4f, 0.2f));
		
		for (int i = 0; i < 6; i++) {
			paramSelectors[i] = std::make_shared<Selector>(textHandler, 1.0f, spriteShader, "assets/textures/ui/button.png", glm::vec2(0.0f, 0.0f), glm::vec2(0.96f, 1.29f));
			SceneManager::getInstance().addObject(paramSelectors[i]);
			(paramSelectors[i])->updateScreenSize(glm::vec2((float)screenSizeI.x, (float)screenSizeI.y));
		}

		selectorPositions[0] = glm::vec2(-0.51f, 0.53f);
		selectorPositions[1] = glm::vec2(-0.51f, 0.0f);
		selectorPositions[2] = glm::vec2(-0.51f, -0.53f);
		selectorPositions[3] = glm::vec2(0.51f, 0.53f);
		selectorPositions[4] = glm::vec2(0.51f, 0.0f);
		selectorPositions[5] = glm::vec2(0.51f, -0.53f);

		(paramSelectors[0])->setup(0.17f, 0.25f, 0.82f, "Eve Delivery Count", glm::vec2(-0.42f + 0.064f, 0.18f), glm::vec2(-0.02f, -0.065f));
		(paramSelectors[1])->setup(0.17f, 0.25f, 0.82f, "Eve Delivery Time", glm::vec2(-0.39f + 0.064f, 0.18f), glm::vec2(-0.02f - 0.043887f, -0.065f));
		(paramSelectors[2])->setup(0.17f, 0.25f, 0.82f, "Splitting Time", glm::vec2(-0.335f + 0.064f, 0.18f), glm::vec2(-0.02f, -0.065f));
		(paramSelectors[3])->setup(0.17f, 0.25f, 0.82f, "MO", glm::vec2(-0.07f, 0.18f), glm::vec2(-0.02f - 0.064365f, -0.065f));
		(paramSelectors[4])->setup(0.17f, 0.25f, 0.82f, "Wall-E Compression Time", glm::vec2(-0.4f - 0.063304f, 0.18f), glm::vec2(-0.02f, -0.065f));
		(paramSelectors[5])->setup(0.17f, 0.25f, 0.82f, "Wall-E Speed Constant", glm::vec2(-0.39f, 0.18f), glm::vec2(-0.02f - 0.043887f, -0.065f));

		(paramSelectors[0])->load(FileManager::getInstance().get(FileManager::CONFIG, "eve_delivery_amount"));
		(paramSelectors[1])->load(str(FileManager::getInstance().get(FileManager::CONFIG, "eve_time_betw_delivery")));
		(paramSelectors[2])->load(
			std::to_string((int)floor((to_float(FileManager::getInstance().get(FileManager::CONFIG, "split_block_time")) + to_float(FileManager::getInstance().get(FileManager::CONFIG, "split_rubbish_time"))) / 2.0f))
		);
		(paramSelectors[3])->load(FileManager::getInstance().get(FileManager::CONFIG, "glb_easy_mode"));
		(paramSelectors[4])->load(std::to_string((int)(floor(to_float(FileManager::getInstance().get(FileManager::CONFIG, "walle_compression_time"))))));
		(paramSelectors[5])->load(str(FileManager::getInstance().get(FileManager::CONFIG, "walle_weight_constant")));

		SceneManager::getInstance().addObject(playButton);
		SceneManager::getInstance().addObject(backButton);

		// timer
		transitionInTimer = make_unique<Timer>(2.0f, []() {}, false);

		transitionOutTimer = make_unique<Timer>(2.0f, [this]() {
			transitionTimeout();
			}, false);

		transitionOutTimer->pause();
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

		backButton->setPosition(glm::vec2(0.296379f, -softInOut - 0.8f + debug.y));
		playButton->setPosition(glm::vec2(-0.296379f, -softInOut - 0.8f + debug.y));

		for (int i = 0; i < 6; i++) {
			if (i < 3)
				(paramSelectors[i])->setPosition(selectorPositions[i] + glm::vec2(-softInOut, 0.0f));
			else
				(paramSelectors[i])->setPosition(selectorPositions[i] + glm::vec2(softInOut, 0.0f));
		}

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

	virtual void windowResizedCallback(GLFWwindow* window, int width, int height) {
		for (int i = 0; i < 6; i++) {
			(paramSelectors[i])->updateScreenSize(glm::vec2((float)width, (float)height));
		}
	}


	virtual void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			double xpos, ypos;
			int width, height;

			glfwGetCursorPos(window, &xpos, &ypos);
			glfwGetWindowSize(window, &width, &height);

			glm::vec2 mousePos((float)xpos / (float)width * 2.0f - 1.0f, -(float)ypos / (float)height * 2.0f + 1.0f);

			if (backButton->isMouseOver(mousePos)) {
				soundPlayer->play2D("assets/audio/ui_click.wav", false);

				// change subscene
				transitionOutTimer->resume();
				nextScene = SceneManager::SceneID::MMDifficultyScene;
			}

			if (playButton->isMouseOver(mousePos)) {
				soundPlayer->play2D("assets/audio/ui_click.wav", false);

				save();
				FileManager::getInstance().save(FileManager::CONFIG);

				SceneManager::getInstance().changeScene(SceneManager::SceneID::GameScene, window);
			}

			(paramSelectors[0])->onClick(mousePos,
				[this](std::string value) {
					soundPlayer->play2D("assets/audio/ui_click.wav", false);

					return std::to_string(cmod(to_int(value) - 1, 11));
				}, [this](std::string value) {
					soundPlayer->play2D("assets/audio/ui_click.wav", false);

					return std::to_string(cmod(to_int(value) + 1, 11));
					});

			(paramSelectors[1])->onClick(mousePos,
				[this](std::string value) {
					soundPlayer->play2D("assets/audio/ui_click.wav", false);

					return str(std::to_string(max(to_float(value) / 2.0f, 0.25f)));
				}, [this](std::string value) {
					soundPlayer->play2D("assets/audio/ui_click.wav", false);

					return str(std::to_string(min(to_float(value) * 2.0f, 2.0f)));
					});

			(paramSelectors[2])->onClick(mousePos,
				[this](std::string value) {
					soundPlayer->play2D("assets/audio/ui_click.wav", false);

					return std::to_string(cmod(to_int(value) - 4 - 2, 10) + 4);
				}, [this](std::string value) {
					soundPlayer->play2D("assets/audio/ui_click.wav", false);

					return std::to_string(cmod(to_int(value) - 4 + 2, 10) + 4);
					});

			(paramSelectors[3])->onClick(mousePos,
				[this](std::string value) {
					soundPlayer->play2D("assets/audio/ui_click.wav", false);

					return (!(to_bool(value))) ? "true" : "false";
				}, [this](std::string value) {
					soundPlayer->play2D("assets/audio/ui_click.wav", false);

					return (!(to_bool(value))) ? "true" : "false";
					});

			(paramSelectors[4])->onClick(mousePos,
				[this](std::string value) {
					soundPlayer->play2D("assets/audio/ui_click.wav", false);

					return std::to_string(cmod(to_int(value) - 1 - 1, 4) + 1);
				}, [this](std::string value) {
					soundPlayer->play2D("assets/audio/ui_click.wav", false);

					return std::to_string(cmod(to_int(value) - 1 + 1, 4) + 1);
					});

			(paramSelectors[5])->onClick(mousePos,
				[this](std::string value) {
					soundPlayer->play2D("assets/audio/ui_click.wav", false);

					return str(std::to_string(floor(cmod((int)(to_float(value) * 10.0f) - 1 - 1, 9) + 1) / 10.0f));
				}, [this](std::string value) {
					soundPlayer->play2D("assets/audio/ui_click.wav", false);

					return str(std::to_string(floor(cmod((int)(to_float(value) * 10.0f) - 1 + 1, 9) + 1) / 10.0f));
					});
		}
	}

	void changeDebugParameters() {
		changeDebugYParameters();

		float velocity = 0.0f;

		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			velocity += 1.0f;

		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			velocity -= 1.0f;

		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			velocity *= 2.0f;

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			velocity /= 4.0f;


		if (velocity == 0.0f) return;

		debug.y += velocity * (float)deltaTime * 2.0f;
		printf("debug: (%f, %f)\n", debug.x, debug.y);
	}

	void changeDebugYParameters() {
		float velocity = 0.0f;

		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			velocity += 1.0f;

		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			velocity -= 1.0f;

		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			velocity *= 2.0f;

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			velocity /= 4.0f;


		if (velocity == 0.0f) return;

		debug.x += velocity * (float)deltaTime * 2.0f;
		printf("debug: (%f, %f)\n", debug.x, debug.y);
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

		playButton->animate(deltaTime, mousePos, [](float x) {
			return sin(x) * 0.05f + 0.95f;
			}, [](float x) {
				return sin(x * 2.0f) * 3.0f;
				}, 3.0f);

		std::function<float(float)> scaleAnim = [](float x) {
			return 1.2f;
			};

		std::function<float(float)> rotStatic = [](float x) {
			return 0.0f;
			};

		for (int i = 0; i < 6; i++) {
			(paramSelectors[i])->animate(deltaTime, mousePos, scaleAnim, rotStatic, 50.0f, 30.0f);
		}
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

	void save() {
		(paramSelectors[0])->save("eve_delivery_amount");
		(paramSelectors[1])->save("eve_time_betw_delivery");
		(paramSelectors[2])->save("split_block_time", "split_rubbish_time");
		(paramSelectors[3])->save("glb_easy_mode");
		(paramSelectors[4])->save("walle_compression_time");
		(paramSelectors[5])->save("walle_weight_constant");
	}

	virtual void end() {
		for (int i = 0; i < 6; i++) {
			(paramSelectors[i]).reset();
		}

		spriteShader.reset();
		PBRShader.reset();

		backButton.reset();
		playButton.reset();

		transitionInTimer.release();
		transitionOutTimer.release();

		SceneManager::getInstance().removeAllObjects();
	}
};
