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
#include "../core/scene.h"

#include "../characters/character.h"
#include "../characters/walle.h"
#include "../characters/eve.h"
#include "../characters/rubbish.h"
#include "../characters/block.h"
#include "../characters/image.h"

#include "../utils.h"

#include "i_menu_subscene.h"

#define INSTANT_BOOT

class MainMenuScene : public Scene {
	private:
		std::shared_ptr<Shader> spriteShader;
		std::shared_ptr<Shader> PBRShader;
		std::shared_ptr<Light> sun1;
		std::shared_ptr<Light> sun2;

		std::shared_ptr<Camera> camera;

		std::shared_ptr<Image> background;

		glm::vec3 bgColor = hex_color("#000000");

		irrklang::ISoundEngine* soundPlayer;

		std::unique_ptr<Timer> bootupTimer;

		float debug = 0.0f;
	public:
#ifdef INSTANT_BOOT
		bool bootedUp = false;
#else
		bool bootedUp = true;
#endif
		MainMenuScene(GLFWwindow* windowRef) : Scene(windowRef) {};

		virtual void init() {
			glDisable(GL_DEPTH_TEST);

			// time initialization
			lastElapsed = glfwGetTime() - offset;
			elapsed = glfwGetTime() - offset;

			// sfx player
			soundPlayer = SceneManager::getInstance().soundManager;

			//bootup
			soundPlayer->play2D("assets/audio/boot_up.wav", false);
			bootupTimer = make_unique<Timer>(2.0f, [this]() {
				bootedUp = true;
				}, false);

			// camera
			SceneManager::getInstance().camera = std::make_shared<Camera>();
			camera = SceneManager::getInstance().camera;
			camera->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));

			// shader
			spriteShader = std::make_shared<Shader>("core/shaders/sprite_shader.vs", "core/shaders/menu_sprite_shader.fs");
			PBRShader = std::make_shared<Shader>("core/shaders/PBR_shader.vs", "core/shaders/PBR_shader.fs");

			// characters
			background = std::make_shared<Image>(spriteShader, "assets/textures/bg_menu_placeholder.png", glm::vec2(0.0f, 0.0f), glm::vec2(2.0f, 2.0f));

			// lights setup
			//sun = std::make_shared<Light>(glm::vec3(getNextRandomRange(-1.0f, 1.0f), getNextRandomRange(-0.8f, 1.0f), getNextRandomRange(-0.3f, 0.3f)), 0.7f, "#ffffff");
			sun1 = std::make_shared<Light>(glm::vec3(-0.81f, -0.29f, 1.97f), 0.7f, "#ffffff");
			sun2 = std::make_shared<Light>(glm::vec3(-0.28f, 0.82f, 0.0f), 2.5f, "#ffffff");
			SceneManager::getInstance().sun = sun1;

			spriteShader->use();
			spriteShader->setInt("mainTexture", 0);
			spriteShader->setMat4("camera", camera->getViewMatrix());
			spriteShader->setVec3("viewPosition", camera->getPosition());
			spriteShader->setVec3("ambient", hex_color("#a1d8e8") * 0.3f);
			spriteShader->setVec3("lights[0].color", sun1->getColor() * sun1->strength);
			spriteShader->setVec3("lights[0].position", sun1->position);

			// view/projection transformations
			int width, height;
			glfwGetWindowSize(window, &width, &height);
			glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

			PBRShader->use();
			PBRShader->setMat4("camera", camera->getViewMatrix());
			PBRShader->setMat4("projection", perspectiveProjection);
			PBRShader->setVec3("viewPosition", camera->getPosition());
			PBRShader->setVec3("ambient", hex_color("#a1d8e8") * 0.3f);
			PBRShader->setVec3("lights[0].color", sun1->getColor() * sun1->strength);
			PBRShader->setVec3("lights[1].color", sun2->getColor() * sun2->strength);
			PBRShader->setVec3("lights[0].position", sun1->position);
			PBRShader->setVec3("lights[1].position", sun2->position);

			SceneManager::getInstance().changeSubscene(SceneManager::getInstance().currentScene, SceneManager::SceneID::MMWelcomeScene, window,
				[this](std::shared_ptr<Scene> newSubscene) {
					std::shared_ptr<IMenuSubscene> theSubscene = std::dynamic_pointer_cast<IMenuSubscene>(newSubscene);
					
					if (theSubscene == nullptr) return;

					theSubscene->spriteShader = spriteShader;
					theSubscene->PBRShader = PBRShader;

					theSubscene->soundPlayer = soundPlayer;
				});
		}

		virtual float update() {
			// reset screen
			glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// deltaTime calculation
			elapsed = glfwGetTime() - offset;
			deltaTime = elapsed - lastElapsed;
			lastElapsed = elapsed;

		#ifndef INSTANT_BOOT
			bootupTimer->updateTimer(deltaTime);

			if (!bootedUp) {
				return (float)deltaTime;
			}
		#endif
			// global update
			camera->update((float)deltaTime);

			// menu juice
			moveSun1Position();
			
			// background
			background->renderSprite();

			// subscene update
			currentSubscene->update();

			return (float)deltaTime;
		}

		virtual void guiUpdate() {
			currentSubscene->guiUpdate();
		}

		virtual void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
			currentSubscene->mouseCallback(window, button, action, mods);
		}

		void moveSun1Position() {
			glm::vec3 m_velocity = glm::vec3(0.0f);

			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
				m_velocity -= glm::vec3(0.0f, 0.0f, 1.0f);

			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				m_velocity += glm::vec3(0.0f, 0.0f, 1.0f);

			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
				m_velocity -= glm::vec3(1.0f, 0.0f, 0.0f);

			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				m_velocity += glm::vec3(1.0f, 0.0f, 0.0f);

			if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
				m_velocity += glm::vec3(0.0f, 1.0f, 0.0f);

			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
				m_velocity -= glm::vec3(0.0f, 1.0f, 0.0f);


			if (m_velocity == glm::vec3(0.0f)) return;


			m_velocity = glm::normalize(m_velocity);

			sun1->position += m_velocity * (float)deltaTime * 2.0f;
			printf("sun1: (%f, %f, %f)\n", sun1->position.x, sun1->position.y, sun1->position.z);

			PBRShader->use();
			PBRShader->setVec3("lights[0].position", sun1->position);
		}

		virtual void windowResizedCallback(GLFWwindow* window, int width, int height) {
			glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

			PBRShader->use();
			PBRShader->setMat4("projection", perspectiveProjection);

			currentSubscene->windowResizedCallback(window, width, height);
		}

		virtual void end() {
			currentSubscene->end();

			spriteShader.reset();
			PBRShader.reset();
			sun1.reset();
			sun2.reset();

			background.reset();

			camera.reset();

			bootupTimer.release();

			SceneManager::getInstance().removeAllObjects();
		}
};
