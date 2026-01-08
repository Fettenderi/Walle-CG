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
		std::shared_ptr<Light> sun;

		//std::shared_ptr<Mo> mo;
		//std::shared_ptr<Eve> eve;
		//std::shared_ptr<Walle> walle;
		std::shared_ptr<Image> walleGuide;
		std::shared_ptr<Image> moGuide;
		std::shared_ptr<Image> startButton;

		std::shared_ptr<Camera> camera;
		std::unique_ptr<Text> guiText;

		std::shared_ptr<Model> rotatingModel;
		std::shared_ptr<Model> logo;

		glm::vec2 startButtonPosition;
		glm::vec2 startButtonScale;


		glm::vec3 bgColor = glm::vec3(0.6f, 0.42f, 0.33f);
	public:

		MainMenuScene() = default;

		virtual void init() {
			// Text Provider
			guiText = std::make_unique<Text>("assets/fonts/Antonio/static/Antonio-Bold.ttf");

			// time initialization
			lastElapsed = glfwGetTime();
			elapsed = glfwGetTime();

			// camera
			SceneManager::getInstance().camera = std::make_shared<Camera>();
			camera = SceneManager::getInstance().camera;


			// shader
			lightedShader = std::make_shared<Shader>("core/shaders/lighted_shader.vs", "core/shaders/lighted_shader.fs");

			
			// background
			std::shared_ptr<Block> background = std::make_shared<Block>(lightedShader, glm::vec2(0.0f, 2.0f), glm::vec2(10.0f, 10.0f));
			background->isPickable = false;
			background->show();

			// characters
			//eve = std::make_shared<Eve>(rubbishPool, lightedShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.28f, 0.4f), 1.0f);
			//walle = std::make_shared<Walle>(rubbishPool, blockPool, lightedShader, glm::vec2(0.0f, 0.0f), glm::vec2(0.4f, 0.4f), 0.0f, 1.0f);
			//mo = make_shared<Mo>(blockPool, lightedShader, glm::vec2(0.0f, -0.6f), glm::vec2(0.28f, 0.4f), 0.0f, 1.0f);
			walleGuide = std::make_shared<Image>(lightedShader, "assets/textures/instructions_walle.png", glm::vec2(-0.65f, -0.5f), glm::vec2(0.5f, 0.8f));
			walleGuide->show();
			moGuide = std::make_shared<Image>(lightedShader, "assets/textures/instructions_mo.png", glm::vec2(0.65f, -0.5f), glm::vec2(0.5f, 0.8f));
			moGuide->show();
			startButton = std::make_shared<Image>(lightedShader, "assets/textures/play_button.png", glm::vec2(0.0f, -0.7f), glm::vec2(0.4f, 0.2f));
			startButton->show();

			startButtonPosition = startButton->getPosition();
			startButtonScale = startButton->getScale();

			//3d models
			rotatingModel = std::make_shared<Model>("assets/models/walle_placeholder.obj");
			logo = std::make_shared<Model>("assets/models/logo.obj");
			

			//SceneManager::getInstance().addObject(background);
			//SceneManager::getInstance().addObject(mo);
			//SceneManager::getInstance().addObject(walle);
			//SceneManager::getInstance().addObject(eve);
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
			//guiText->RenderText(std::format("Collected trash: {}", walle->getCollected()), glm::vec2(0.0f, 0.0f), 1.0f, "#0a1518");



			// view/projection transformations
			glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)600, 0.1f, 100.0f);
			
			

			// rendering the loaded models
			glm::mat4 rotatingModelMat = glm::mat4(1.0f);
			rotatingModelMat = glm::translate(rotatingModelMat, glm::vec3(0.0f, 0.1f, 0.0f)); // translate it down so it's at the center of the scene
			rotatingModelMat = glm::scale(rotatingModelMat, glm::vec3(0.10f, 0.15f, 0.15f));	// it's a bit too big for our scene, so scale it down
			rotatingModelMat = glm::rotate(rotatingModelMat, glm::radians((float)elapsed*10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 logoModelMat = glm::mat4(1.0f);
			logoModelMat = glm::translate(logoModelMat, glm::vec3(0.0f, 0.7f, 0.5f)); // translate it down so it's at the center of the scene
			logoModelMat = glm::scale(logoModelMat, glm::vec3(0.15f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
			logoModelMat = glm::rotate(logoModelMat, glm::radians((float)sin(elapsed/2)*5.0f), glm::vec3(0.0f, 0.5f, 1.0f));
			

			// global update
			camera->update((float)deltaTime);

			// shader update
			//for (std::shared_ptr<Shader> shader : shaders) {
			//    shader->use();
			//    shader->setMat4("camera", camera->getViewMatrix());
			//}

			lightedShader->use();

			lightedShader->setMat4("camera", camera->getViewMatrix3d());
			lightedShader->setMat4("projection", projection);
			lightedShader->setVec3("sunPosition", sun->position);
			lightedShader->setVec3("viewPosition", camera->getPosition());
			
			lightedShader->setMat4("model", rotatingModelMat);
			rotatingModel->Draw(*lightedShader);
			lightedShader->setMat4("model", logoModelMat);
			logo->Draw(*lightedShader);
			
			
			//glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_2D, 0);

			

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

				

				if (isMouseOver(glm::vec2(scX, scY))) {
					SceneManager::getInstance().changeScene(std::make_shared<GameScene>());
				}



				//destinazione di Mo
				//mo->setTarget(glm::vec2(scX, scY) - camera->getPosition2D());
				//printf("MO target: (%f, %f)\n", scX, scY);
			}
		}

		bool isMouseOver(const glm::vec2& mousePos) const
		{
			float halfW = startButtonScale.x * 0.5f;
			float halfH = startButtonScale.y * 0.5f;

			return mousePos.x >= startButtonPosition.x - halfW &&
				mousePos.x <= startButtonPosition.x + halfW &&
				mousePos.y >= startButtonPosition.y - halfH &&
				mousePos.y <= startButtonPosition.y + halfH;
		}

		virtual void end() {
			SceneManager::getInstance().removeAllObjects();
		}
};

#endif
