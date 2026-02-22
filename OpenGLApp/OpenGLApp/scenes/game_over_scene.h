#ifndef GAME_OVER_SCENE_H
#define GAME_OVER_SCENE_H

#include "../core/scene.h"
#include "../core/text.h"
#include "../core/model.h"

#include "../globals/scene_manager.h"

#include "../characters/image.h"

class GameOverScene : public Scene {
	private:
		std::shared_ptr<Shader> spriteShader;
		std::shared_ptr<Shader> PBRShader;

		std::shared_ptr<Light> spotLight1;
		std::shared_ptr<Light> spotLight2;

		std::shared_ptr<Image> mouseDetector;

		std::shared_ptr<Image> menuButton;
		std::shared_ptr<Image> startButton;

		std::unique_ptr<Text> guiText;

		std::shared_ptr<Camera> camera;

		std::shared_ptr<Model> walle;

		glm::vec3 bgColor = hex_color("#000000");

		float elapsedOverStart = 0.0f;
		float elapsedOverMenu = 0.0f;

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
			soundPlayer->play2D("assets/audio/game_over.wav", false);

			// text initialization
			guiText = std::make_unique<Text>("assets/fonts/Antonio/static/Antonio-Bold.ttf");

			// camera
			SceneManager::getInstance().camera = std::make_shared<Camera>();
			camera = SceneManager::getInstance().camera;
			camera->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));

			// shader
			spriteShader = std::make_shared<Shader>("core/shaders/sprite_shader.vs", "core/shaders/menu_sprite_shader.fs");
			PBRShader = std::make_shared<Shader>("core/shaders/PBR_shader.vs", "core/shaders/PBR_shader.fs");

			// characters
			mouseDetector = std::make_shared<Image>(spriteShader, "assets/textures/bg_menu_placeholder.png", glm::vec2(0.0f, 0.227f), glm::vec2(1.134f, 1.258f));

			menuButton = std::make_shared<Image>(spriteShader, "assets/textures/play_button.png", glm::vec2(-0.414f, -0.7f), glm::vec2(0.4f, 0.2f));
			startButton = std::make_shared<Image>(spriteShader, "assets/textures/play_button.png", glm::vec2(0.414f, -0.7f), glm::vec2(0.4f, 0.2f));
			
			//3d models
			walle = std::make_shared<Model>("assets/models/walle/walle-sad.gltf");
			
			SceneManager::getInstance().addObject(menuButton);
			SceneManager::getInstance().addObject(startButton);

			// lights setup
			//sun = std::make_shared<Light>(glm::vec3(getNextRandomRange(-1.0f, 1.0f), getNextRandomRange(-0.8f, 1.0f), getNextRandomRange(-0.3f, 0.3f)), 0.7f, "#ffffff");
			spotLight1 = std::make_shared<Light>(glm::vec3(-0.62f, 2.75f, 0.72f), 10.7f, "#ffffff");
			spotLight2 = std::make_shared<Light>(glm::vec3(-0.62f, 2.75f, 0.72f), 0.0f, "#ffffff");
			SceneManager::getInstance().sun = spotLight1;

			spriteShader->use();
			spriteShader->setInt("mainTexture", 0);
			spriteShader->setMat4("camera", camera->getViewMatrix());
			spriteShader->setVec3("viewPosition", camera->getPosition());
			spriteShader->setVec3("ambient", hex_color("#a1d8e8") * 0.3f);
			spriteShader->setVec3("lights[0].color", spotLight1->getColor() * spotLight1->strength);
			spriteShader->setVec3("lights[1].color", spotLight2->getColor() * spotLight2->strength);

			// view/projection transformations
			int width, height;
			glfwGetWindowSize(window, &width, &height);
			glm::mat4 perspectiveProjection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

			PBRShader->use();
			PBRShader->setMat4("camera", camera->getViewMatrix());
			PBRShader->setMat4("projection", perspectiveProjection);
			PBRShader->setVec3("viewPosition", camera->getPosition());
			PBRShader->setVec3("ambient", hex_color("#a1d8e8") * 0.3f);
			PBRShader->setVec3("lights[0].color", spotLight1->getColor() * spotLight1->strength);
			PBRShader->setVec3("lights[1].color", spotLight2->getColor() * spotLight2->strength);
		}

		virtual float update() {
			// debug
			changeDebugParameters();

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
			float rotationFrequency = (float)PI / 5.0f;

			glm::mat4 walleModelMat = glm::mat4(1.0f);
			walleModelMat = glm::translate(walleModelMat, glm::vec3(0.0f, -0.13f, -0.85f));
			walleModelMat = glm::scale(walleModelMat, glm::vec3(0.042f));
			walleModelMat = glm::rotate(walleModelMat, glm::radians(22.0f * sin((float)elapsed * rotationFrequency)), glm::vec3(0.0f, 1.0f, 0.0f));
						
			glEnable(GL_DEPTH_TEST);

			// models update
			spriteShader->use();
			spriteShader->setVec3("lights[0].position", spotLight1->position);
			spriteShader->setVec3("lights[1].position", spotLight2->position);

			PBRShader->use();
			PBRShader->setVec3("lights[0].position", spotLight1->position);
			PBRShader->setVec3("lights[1].position", spotLight2->position);

			PBRShader->setMat4("model", walleModelMat);
			walle->Draw(*PBRShader);
			glDisable(GL_DEPTH_TEST);

			return (float)deltaTime;
		}

		float debug = 0.0f;

		virtual void guiUpdate() {
			int width, height;
			glfwGetWindowSize(window, &width, &height);

			guiText->RenderText(std::format("Game Over"), glm::vec2(width / 2.0f -202.0f, height / 2.0f + 118.0f + debug), 2.0f, "#0a1518");
			//guiText->RenderText(std::format("{:02.0f}:{:02.0f}", , ), glm::vec2(width / 2.0f - 34.0f, height / 2.0f), 3.0f, "#0a1518");
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

				if (menuButton->isMouseOver(glm::vec2(scX, scY))) {
					soundPlayer->play2D("assets/audio/ui_click.wav", false);
					SceneManager::getInstance().changeScene(SceneManager::SceneID::MainMenuScene, window);
				}
			}
		}

		void moveLightPosition() {
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

			spotLight2->position += m_velocity * (float)deltaTime * 2.0f;
			printf("spotLight: (%f, %f, %f)\n", spotLight2->position.x, spotLight2->position.y, spotLight2->position.z);
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
				velocity /= 2.0f;


			if (velocity == 0.0f) return;

			debug += velocity * (float)deltaTime * 10.0f;
			printf("debug: (%f)\n", debug);
		}

		void buttonUpdate() {
			double xpos, ypos;
			int width, height;

			glfwGetCursorPos(window, &xpos, &ypos);
			glfwGetWindowSize(window, &width, &height);

			float scX = (float)xpos / (float)width * 2.0f - 1.0f;
			float scY = -(float)ypos / (float)height * 2.0f + 1.0f;

			if (startButton->isMouseOver(glm::vec2(scX, scY))) {
				elapsedOverStart += (float)deltaTime;
				startButton->setScale(explerp(startButton->getScale(), sin(elapsedOverStart) * 0.05f + 0.95f, (float)deltaTime * 3.0f));
				startButton->setRotation(explerp(startButton->getRotation(), sin(elapsedOverStart * 2.0f) * 3.0f, (float)deltaTime * 3.0f));
			}
			else {
				startButton->setScale(explerp(startButton->getScale(), 1.0f, (float)deltaTime * 1.0f));
				startButton->setRotation(explerp(startButton->getRotation(), 0.0f, (float)deltaTime * 1.0f));

				elapsedOverStart = explerp(elapsedOverStart, 0.0f, (float)deltaTime * 1.0f);
			}

			if (menuButton->isMouseOver(glm::vec2(scX, scY))) {
				elapsedOverMenu += (float)deltaTime;
				menuButton->setScale(explerp(menuButton->getScale(), sin(elapsedOverMenu) * 0.05f + 0.95f, (float)deltaTime * 3.0f));
				menuButton->setRotation(explerp(menuButton->getRotation(), sin(elapsedOverMenu * 2.0f) * 3.0f, (float)deltaTime * 3.0f));
			}
			else {
				menuButton->setScale(explerp(menuButton->getScale(), 1.0f, (float)deltaTime * 1.0f));
				menuButton->setRotation(explerp(menuButton->getRotation(), 0.0f, (float)deltaTime * 1.0f));

				elapsedOverMenu = explerp(elapsedOverMenu, 0.0f, (float)deltaTime * 1.0f);
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
			spotLight1.reset();
			spotLight2.reset();

			mouseDetector.reset();

			startButton.reset();

			camera.reset();

			walle.reset();

			//soundPlayer;

			SceneManager::getInstance().removeAllObjects();
		}
};

#endif
