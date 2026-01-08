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
#include "../scenes/game_scene.h"

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

class MainMenuScene : public Scene {
	private:
		std::shared_ptr<Shader> lightedShader;
		std::shared_ptr<Shader> modelShader;
		std::shared_ptr<Light> sun;

		std::shared_ptr<Image> background;
		std::shared_ptr<Image> walleGuide;
		std::shared_ptr<Image> moGuide;
		std::shared_ptr<Image> startButton;

		std::shared_ptr<Camera> camera;

		std::shared_ptr<Model> rotatingModel;
		std::shared_ptr<Model> logo;

		glm::vec3 bgColor = glm::vec3(0.6f, 0.42f, 0.33f);

		float elapsedOver = 0.0f;
	public:

		MainMenuScene(GLFWwindow* windowRef) : Scene(windowRef) {};

		virtual void init() {
			// time initialization
			lastElapsed = glfwGetTime();
			elapsed = glfwGetTime();

			// camera
			SceneManager::getInstance().camera = std::make_shared<Camera>();
			camera = SceneManager::getInstance().camera;

			// shader
			lightedShader = std::make_shared<Shader>("core/shaders/lighted_shader.vs", "core/shaders/lighted_shader.fs");
			modelShader = std::make_shared<Shader>("core/shaders/model_shader.vs", "core/shaders/model_shader.fs");

			// characters
			background = std::make_shared<Image>(lightedShader, "assets/textures/block.png", glm::vec2(0.0f, 0.0f), glm::vec2(10.0f, 10.0f));
			walleGuide = std::make_shared<Image>(lightedShader, "assets/textures/instructions_walle.png", glm::vec2(-0.65f, -0.5f), glm::vec2(0.5f, 0.8f));
			moGuide = std::make_shared<Image>(lightedShader, "assets/textures/instructions_mo.png", glm::vec2(0.65f, -0.5f), glm::vec2(0.5f, 0.8f));
			startButton = std::make_shared<Image>(lightedShader, "assets/textures/play_button.png", glm::vec2(0.0f, -0.7f), glm::vec2(0.4f, 0.2f));

			//3d models
			rotatingModel = std::make_shared<Model>("assets/models/mela/mela.obj");
			logo = std::make_shared<Model>("assets/models/logo.obj");
			
			SceneManager::getInstance().addObject(walleGuide);
			SceneManager::getInstance().addObject(moGuide);
			SceneManager::getInstance().addObject(startButton);

			// lights setup
			sun = std::make_shared<Light>(glm::vec3(0.0f, 0.0f, 5.0f), 0.7f, "#ffffff");
			SceneManager::getInstance().sun = sun;

			lightedShader->use();
			lightedShader->setInt("mainTexture", 0);

			lightedShader->setVec3("ambientColor", hex_color("#a1d8e8"));
			lightedShader->setVec3("sunColor", sun->getColor());
			lightedShader->setFloat("sunStrength", sun->strength);

			modelShader->use();
			modelShader->setInt("diffuseTexture", 0);

			modelShader->setVec3("ambientColor", hex_color("#a1d8e8"));
			modelShader->setVec3("sunColor", sun->getColor());
			modelShader->setFloat("sunStrength", sun->strength);
		}

		virtual float update() {
			// reset screen
			glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// deltaTime calculation
			elapsed = glfwGetTime();
			deltaTime = elapsed - lastElapsed;
			lastElapsed = elapsed;

			// view/projection transformations
			glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)600, 0.1f, 100.0f);
			
			// rendering the loaded models
			glm::mat4 rotatingModelMat = glm::mat4(1.0f);
			rotatingModelMat = glm::translate(rotatingModelMat, glm::vec3(0.0f, 0.0f, -2.0f));
			rotatingModelMat = glm::scale(rotatingModelMat, glm::vec3(0.10f, 0.15f, 0.15f));
			rotatingModelMat = glm::rotate(rotatingModelMat, glm::radians((float)elapsed * 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			rotatingModelMat = glm::rotate(rotatingModelMat, glm::radians(-12.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			
			glm::mat4 logoModelMat = glm::mat4(1.0f);
			logoModelMat = glm::translate(logoModelMat, glm::vec3(0.0f, 0.4f, -1.7f));
			logoModelMat = glm::scale(logoModelMat, glm::vec3(0.15f, 0.2f, 0.2f));
			logoModelMat = glm::rotate(logoModelMat, glm::radians((float)sin(elapsed * 2.0f) * 5.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			logoModelMat = glm::rotate(logoModelMat, glm::radians((float)cos(elapsed * 2.0f) * 5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			
			// global update
			camera->update((float)deltaTime);

			// models update
			lightedShader->use();
			lightedShader->setMat4("camera", camera->getViewMatrix());
			lightedShader->setVec3("sunPosition", sun->position);
			lightedShader->setVec3("viewPosition", camera->getPosition());

			modelShader->use();
			modelShader->setMat4("camera", camera->getViewMatrix());
			modelShader->setMat4("projection", projection);
			modelShader->setVec3("sunPosition", sun->position);
			modelShader->setVec3("viewPosition", camera->getPosition());

			modelShader->setMat4("model", rotatingModelMat);
			rotatingModel->Draw(*modelShader);

			modelShader->setMat4("model", logoModelMat);
			logo->Draw(*modelShader);
			
			// button update
			buttonUpdate();

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

				if (startButton->isMouseOver(glm::vec2(scX, scY))) {
					SceneManager::getInstance().changeScene(std::make_shared<GameScene>(window));
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

			if (startButton->isMouseOver(glm::vec2(scX, scY))) {
				elapsedOver += deltaTime;
				startButton->setScale(explerp(startButton->getScale(), sin(elapsedOver) * 0.05f + 0.95f, deltaTime * 3.0f));
				startButton->setRotation(explerp(startButton->getRotation(), sin(elapsedOver * 2.0f) * 3.0f, deltaTime * 3.0f));
			}
			else {
				startButton->setScale(explerp(startButton->getScale(), 1.0f, deltaTime * 1.0f));
				startButton->setRotation(explerp(startButton->getRotation(), 0.0f, deltaTime * 1.0f));

				elapsedOver = explerp(elapsedOver, 0.0f, deltaTime * 1.0f);
			}
		}

		virtual void end() {
			SceneManager::getInstance().removeAllObjects();
		}
};

#endif
