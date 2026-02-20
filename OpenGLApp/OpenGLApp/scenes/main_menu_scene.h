#ifndef MAIN_MENU_SCENE_H
#define MAIN_MENU_SCENE_H

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

#define INSTANT_BOOT

class MainMenuScene : public Scene {
	private:
		std::shared_ptr<Shader> spriteShader;
		std::shared_ptr<Shader> PBRShader;
		std::shared_ptr<Light> sun1;
		std::shared_ptr<Light> sun2;

		std::shared_ptr<Image> mouseDetector;

		std::shared_ptr<Image> background;
		std::shared_ptr<Image> walleGuide;
		std::shared_ptr<Image> moGuide;
		std::shared_ptr<Image> startButton;

		std::shared_ptr<Camera> camera;

		std::shared_ptr<Model> walle;
		std::shared_ptr<Model> logo;

		glm::vec3 bgColor = hex_color("#000000");

		float elapsedOver = 0.0f;

		std::shared_ptr<Image> grabbedImage;
		bool hasGrabbedImage = false;
		bool isFirstFrame = true;

		irrklang::ISoundEngine* soundPlayer;

		std::unique_ptr<Timer> bootupTimer;

		glm::vec2 previousMousePos = glm::vec2(0.0f);
		glm::vec2 spinVelocity = glm::vec2(0.0f);

		float modelOffset = 0.0f;
		float logoOffset = 0.0f;

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
			mouseDetector = std::make_shared<Image>(spriteShader, "assets/textures/bg_menu_placeholder.png", glm::vec2(0.0f, 0.227f), glm::vec2(1.134f, 1.258f));

			background = std::make_shared<Image>(spriteShader, "assets/textures/bg_menu_placeholder.png", glm::vec2(0.0f, 0.0f), glm::vec2(2.0f, 2.0f));
			walleGuide = std::make_shared<Image>(spriteShader, "assets/textures/instructions_walle.png", glm::vec2(-0.657143f, -0.490476f), glm::vec2(0.5f, 0.8f));
			moGuide = std::make_shared<Image>(spriteShader, "assets/textures/instructions_mo.png", glm::vec2(0.691071f, -0.554762f), glm::vec2(0.5f, 0.8f));
			startButton = std::make_shared<Image>(spriteShader, "assets/textures/play_button.png", glm::vec2(0.0f, -0.7f), glm::vec2(0.4f, 0.2f));
			
			//3d models
			walle = std::make_shared<Model>("assets/models/walle/walle.gltf");
			logo = std::make_shared<Model>("assets/models/logo/logo.obj");
			
			SceneManager::getInstance().addObject(walleGuide);
			SceneManager::getInstance().addObject(moGuide);
			SceneManager::getInstance().addObject(startButton);

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
			handleMouseModelInteraction();
			moveSun1Position();
			//moveSun2Position();
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
			
			// background
			background->renderSprite();
			glEnable(GL_DEPTH_TEST);

			// models update
			spriteShader->use();
			spriteShader->setVec3("lights[0].position", sun1->position);

			PBRShader->use();
			PBRShader->setVec3("lights[0].position", sun1->position);
			PBRShader->setVec3("lights[1].position", sun2->position);

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

				if (startButton->isMouseOver(glm::vec2(scX, scY))) {
					soundPlayer->play2D("assets/audio/ui_click.wav", false);
					SceneManager::getInstance().changeScene(SceneManager::SceneID::GameScene, window);
				}
			}
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
		}

		void moveSun2Position() {
			glm::vec3 m_velocity = glm::vec3(0.0f);

			if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
				m_velocity -= glm::vec3(0.0f, 0.0f, 1.0f);

			if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
				m_velocity += glm::vec3(0.0f, 0.0f, 1.0f);

			if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
				m_velocity -= glm::vec3(1.0f, 0.0f, 0.0f);

			if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
				m_velocity += glm::vec3(1.0f, 0.0f, 0.0f);

			if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
				m_velocity += glm::vec3(0.0f, 1.0f, 0.0f);

			if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
				m_velocity -= glm::vec3(0.0f, 1.0f, 0.0f);


			if (m_velocity == glm::vec3(0.0f)) return;


			m_velocity = glm::normalize(m_velocity);

			sun2->position += m_velocity * (float)deltaTime * 2.0f;
			printf("sun2: (%f, %f, %f)\n", sun2->position.x, sun2->position.y, sun2->position.z);
		}


		void moveCameraPosition() {
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

			camera->setPosition(camera->getPosition() + m_velocity * (float)deltaTime * 2.0f);
			printf("(%f, %f, %f)\n", camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
		}

		void buttonUpdate() {
			double xpos, ypos;
			int width, height;

			glfwGetCursorPos(window, &xpos, &ypos);
			glfwGetWindowSize(window, &width, &height);

			float scX = (float)xpos / (float)width * 2.0f - 1.0f;
			float scY = -(float)ypos / (float)height * 2.0f + 1.0f;

			if (startButton->isMouseOver(glm::vec2(scX, scY))) {
				elapsedOver += (float)deltaTime;
				startButton->setScale(explerp(startButton->getScale(), sin(elapsedOver) * 0.05f + 0.95f, (float)deltaTime * 3.0f));
				startButton->setRotation(explerp(startButton->getRotation(), sin(elapsedOver * 2.0f) * 3.0f, (float)deltaTime * 3.0f));
			}
			else {
				startButton->setScale(explerp(startButton->getScale(), 1.0f, (float)deltaTime * 1.0f));
				startButton->setRotation(explerp(startButton->getRotation(), 0.0f, (float)deltaTime * 1.0f));

				elapsedOver = explerp(elapsedOver, 0.0f, (float)deltaTime * 1.0f);
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
			sun1.reset();
			sun2.reset();

			mouseDetector.reset();

			background.reset();
			walleGuide.reset();
			moGuide.reset();
			startButton.reset();

			camera.reset();

			walle.reset();
			logo.reset();

			grabbedImage.reset();
			bootupTimer.release();

			SceneManager::getInstance().removeAllObjects();
		}
};

#endif
