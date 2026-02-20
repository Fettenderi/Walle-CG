#ifndef GAME_OVER_SCENE_H
#define GAME_OVER_SCENE_H

#include "../core/scene.h"
#include "../globals/scene_manager.h"

#include "../core/model.h"

#include "../characters/image.h"

class GameOverScene : public Scene {
	private:
		std::shared_ptr<Shader> spriteShader;
		std::shared_ptr<Shader> PBRShader;

		std::shared_ptr<Light> spotLight;

		std::shared_ptr<Image> mouseDetector;

		std::shared_ptr<Image> background;
		std::shared_ptr<Image> startButton;

		std::shared_ptr<Camera> camera;

		std::shared_ptr<Model> walle;

		glm::vec3 bgColor = hex_color("#000000");

		float elapsedOver = 0.0f;

		irrklang::ISoundEngine* soundPlayer;

	public:
		GameOverScene(GLFWwindow* windowRef) : Scene(windowRef) {};

		virtual void init() {
			glDisable(GL_DEPTH_TEST);

			// time initialization
			lastElapsed = glfwGetTime() - offset;
			elapsed = glfwGetTime() - offset;

			// sfx player
			soundPlayer = SceneManager::getInstance().soundManager;

			//bootup
			soundPlayer->play2D("assets/audio/boot_up.wav", false);

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
			startButton = std::make_shared<Image>(spriteShader, "assets/textures/play_button.png", glm::vec2(0.0f, -0.7f), glm::vec2(0.4f, 0.2f));
			
			//3d models
			walle = std::make_shared<Model>("assets/models/walle/walle.gltf");
			
			SceneManager::getInstance().addObject(startButton);

			// lights setup
			//sun = std::make_shared<Light>(glm::vec3(getNextRandomRange(-1.0f, 1.0f), getNextRandomRange(-0.8f, 1.0f), getNextRandomRange(-0.3f, 0.3f)), 0.7f, "#ffffff");
			spotLight = std::make_shared<Light>(glm::vec3(-0.81f, -0.29f, 1.97f), 0.7f, "#ffffff");
			SceneManager::getInstance().sun = spotLight;

			spriteShader->use();
			spriteShader->setInt("mainTexture", 0);
			spriteShader->setMat4("camera", camera->getViewMatrix());
			spriteShader->setVec3("viewPosition", camera->getPosition());
			spriteShader->setVec3("ambient", hex_color("#a1d8e8") * 0.3f);
			spriteShader->setVec3("lights[0].color", spotLight->getColor() * spotLight->strength);

			// view/projection transformations
			int width, height;
			glfwGetWindowSize(window, &width, &height);
			glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

			PBRShader->use();
			PBRShader->setMat4("camera", camera->getViewMatrix());
			PBRShader->setMat4("projection", perspectiveProjection);
			PBRShader->setVec3("viewPosition", camera->getPosition());
			PBRShader->setVec3("ambient", hex_color("#a1d8e8") * 0.3f);
			PBRShader->setVec3("lights[0].color", spotLight->getColor() * spotLight->strength);
		}

		virtual float update() {
			// reset screen
			glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// deltaTime calculation
			elapsed = glfwGetTime() - offset;
			deltaTime = elapsed - lastElapsed;
			lastElapsed = elapsed;

			// global update
			camera->update((float)deltaTime);

			// menu juice
			buttonUpdate();
			
			// rendering the loaded models
			glm::mat4 walleModelMat = glm::mat4(1.0f);
			walleModelMat = glm::translate(walleModelMat, glm::vec3(0.0f, -0.13f, -0.85f));
			walleModelMat = glm::scale(walleModelMat, glm::vec3(0.072f));
			walleModelMat = glm::rotate(walleModelMat, glm::radians((float)elapsed * 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
						
			// background
			background->renderSprite();
			glEnable(GL_DEPTH_TEST);

			// models update
			spriteShader->use();
			spriteShader->setVec3("lights[0].position", spotLight->position);

			PBRShader->use();
			PBRShader->setVec3("lights[0].position", spotLight->position);

			PBRShader->setMat4("model", walleModelMat);
			walle->Draw(*PBRShader);
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

				if (startButton->isMouseOver(glm::vec2(scX, scY))) {
					soundPlayer->play2D("assets/audio/ui_click.wav", false);
					SceneManager::getInstance().changeScene(SceneManager::SceneID::GameScene, window);
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

		virtual void windowResizedCallback(GLFWwindow* window, int width, int height) {
			glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
		
			PBRShader->use();
			PBRShader->setMat4("projection", perspectiveProjection);
		}

		virtual void end() {
			spriteShader.reset();
			PBRShader.reset();
			spotLight.reset();

			mouseDetector.reset();

			background.reset();
			startButton.reset();

			camera.reset();

			walle.reset();

			//soundPlayer;

			SceneManager::getInstance().removeAllObjects();
		}
};

#endif
