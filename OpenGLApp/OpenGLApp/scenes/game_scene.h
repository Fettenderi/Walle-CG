#ifndef GAME_SCENE_H
#define GAME_SCENE_H

#include "../core/scene.h"
#include "../globals/scene_manager.h"

#include "../core/pool.h"
#include "../core/timer.h"
#include "../core/text.h"

#include "../characters/character.h"
#include "../characters/walle.h"
#include "../characters/eve.h"
#include "../characters/mo.h"
#include "../characters/rubbish.h"
#include "../characters/block.h"

#define PI 3.14159265358979323846

class GameScene : public Scene {
	private:
		std::shared_ptr<Shader> spriteShader;
		std::shared_ptr<Light> sun;

		std::shared_ptr<Image> background;
		std::shared_ptr<Mo> mo;
		std::shared_ptr<Eve> eve;
		std::shared_ptr<Walle> walle;

		std::shared_ptr<Camera> camera;
		std::unique_ptr<Text> guiText;

		std::shared_ptr<ObjectPool<Rubbish>> rubbishPool;
		std::shared_ptr<ObjectPool<Block>> blockPool;

		glm::vec3 bgColor = glm::vec3(0.6f, 0.42f, 0.33f);

		float debug;

		float currentDayNightFrequency = 0.0f;
		float dayNightPhase = 0.0f;

		bool lmbPressed = false;

		int maxRubbish;

		void handleDayNightCycle() {
			float totalTime = 500.0f;
			float nightPercentage = 0.3f;
			float newDayNightFrequency;

			float time = elapsed / totalTime - floor(elapsed / totalTime);

			if (time <= (1.0f - nightPercentage)) {
				// day
				newDayNightFrequency = (float)PI / (totalTime * (1.0f - nightPercentage));
				walle->setFlashlight(false);

			} else {
				// night
				newDayNightFrequency = (float)PI / (totalTime * nightPercentage);
				walle->setFlashlight(true);
			}

			// When changing frequency you need to add a phase in order to allign 
			// to the next valid y value of the new trig function
			if (currentDayNightFrequency != newDayNightFrequency) {
				dayNightPhase = (currentDayNightFrequency - newDayNightFrequency) * (float)elapsed + dayNightPhase;
				currentDayNightFrequency = newDayNightFrequency;
			}

			debug = time;

			sun->position = glm::vec3(7.0f * glm::vec4(-cos(elapsed * newDayNightFrequency + dayNightPhase), 0.0f, sin(elapsed * newDayNightFrequency + dayNightPhase), 1.0f) *
				glm::rotate(glm::mat4(1.0f), glm::radians(11.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
			
			
		}

	public:

		GameScene(GLFWwindow* windowRef) : Scene(windowRef) {};

		virtual void init() {
			glDisable(GL_DEPTH_TEST);

			// loading config
			maxRubbish = 5;

			// text initialization
			guiText = std::make_unique<Text>("assets/fonts/Antonio/static/Antonio-Bold.ttf");

			// time initialization
			lastElapsed = glfwGetTime() - offset;
			elapsed = glfwGetTime() - offset;

			// camera
			SceneManager::getInstance().camera = std::make_shared<Camera>();
			camera = SceneManager::getInstance().camera;

			camera->setSpeed(0.4f);
			camera->setMoving(false);

			// shader
			spriteShader = std::make_shared<Shader>("core/shaders/sprite_shader.vs", "core/shaders/ingame_sprite_shader.fs");

			// pools initialization
			rubbishPool = std::make_shared<ObjectPool<Rubbish>>(10, spriteShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.4f, 0.32f));
			blockPool = std::make_shared<ObjectPool<Block>>(30, spriteShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.25f, 0.25f));

			// background
			background = std::make_shared<Image>(spriteShader, "assets/textures/bg_placeholder.png", glm::vec2(0.0f, 0.0f), glm::vec2(2.0f, 2.0f));

			// characters
			eve = std::make_shared<Eve>(rubbishPool, spriteShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.28f, 0.4f), 1.3f);
			walle = std::make_shared<Walle>(rubbishPool, blockPool, spriteShader, glm::vec2(0.0f, 0.0f), glm::vec2(0.4f, 0.4f), 0.0f, 1.0f);
			mo = make_shared<Mo>(blockPool, spriteShader, glm::vec2(0.0f, -0.6f), glm::vec2(0.28f, 0.4f), 0.0f, 1.0f);

			SceneManager::getInstance().addObject(mo);
			SceneManager::getInstance().addObject(walle);
			SceneManager::getInstance().addObject(eve);

			// lights setup
			sun = std::make_shared<Light>(glm::vec3(1.0f, 0.0f, 0.0f), 80.0f, "#ffffab");
			SceneManager::getInstance().sun = sun;
			SceneManager::getInstance().ambientStrength = 0.3f;


			spriteShader->use();
			spriteShader->setInt("mainTexture", 0);

			spriteShader->setMat4("camera", camera->getViewMatrix());
			spriteShader->setVec3("viewPosition", camera->getPosition());

			spriteShader->setVec3("ambient", hex_color("#a1d8e8") * SceneManager::getInstance().ambientStrength);

			spriteShader->setVec3("lights[0].color", sun->getColor() * sun->strength);
			spriteShader->setVec3("lights[0].position", sun->position);
		}

		virtual float update() {
			// reset screen
			glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// deltaTime calculation
			//elapsed = glfwGetTime() * 1.75f;
			elapsed = glfwGetTime() - offset;
			deltaTime = elapsed - lastElapsed;
			lastElapsed = elapsed;

			// global update
			if (false){//StatsManager::getInstance().currentRubbish > maxRubbish) {
				StatsManager::getInstance().currentRubbish = 0;
				SceneManager::getInstance().changeScene(SceneManager::SceneID::GameOverScene, window);
				return (float)deltaTime;
			}
			
			camera->update((float)deltaTime);
			background->setPosition(camera->getPosition2D());
			background->renderSprite();

			handleDayNightCycle();

			spriteShader->use();
			spriteShader->setMat4("camera", camera->getViewMatrix());
			spriteShader->setVec3("lights[0].position", sun->position);
			spriteShader->setVec3("lights[0].color", sun->getColor() * sun->strength);
			spriteShader->setVec3("viewPosition", camera->getPosition());

			// mo movement
			if (lmbPressed) {
				double xpos, ypos;
				int width, height;

				glfwGetCursorPos(window, &xpos, &ypos);
				glfwGetWindowSize(window, &width, &height);

				//coordinate x schermata
				float scX = (float)xpos / (float)width * 2.0f - 1.0f;
				float scY = -(float)ypos / (float)height * 2.0f + 1.0f;

				//destinazione di Mo
				mo->setTarget(glm::vec2(scX, scY) + camera->getPosition2D());
			}

			return (float)deltaTime;
		}

		virtual void guiUpdate() {
			int width, height;
			glfwGetWindowSize(window, &width, &height);
			
			guiText->RenderText(std::format("{:02.0f}:{:02.0f}", floor((float)elapsed / 60.0f), mod((float)elapsed, 60.0f)), glm::vec2(width / 2.0f - 34.0f, height - 40.0f), 0.7f, "#0a1518");
			guiText->RenderText(std::format("Blocks: {}", StatsManager::getInstance().collectedBlocks), glm::vec2(10.0f, height - 40.0f), 0.7f, "#0a1518");
			guiText->RenderText(std::format("Current Rubbish: {}", StatsManager::getInstance().currentRubbish), glm::vec2(10.0f, 50.0f), 0.7f, "#0a1518");

			//glm::vec2 camPosition = SceneManager::getInstance().camera->getPosition2D();
			//float pileHeight = StatsManager::getInstance().maxBlockProgress;
			//
			//glm::vec2 min = glm::vec2(-0.8f + camPosition.x, remap(-0.8f, -1.0f, 1.0f, fmax(-1.0f - camPosition.y, pileHeight), 1.0f - camPosition.y));
			//glm::vec2 max = glm::vec2(0.8f + camPosition.x, remap(0.8f, -1.0f, 1.0f, fmax(-1.0f - camPosition.y, pileHeight), 1.0f - camPosition.y));
			//guiText->RenderText(std::format("Min: ({:.2f}, {:.2f}), Max: ({:.2f}, {:.2f})", min.x, min.y, max.x, max.y), glm::vec2(10.0f, 10.0f), 0.7f, "#0a1518");
		}


		virtual void mouseCallback(GLFWwindow* passedWindow, int button, int action, int mods) {
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				lmbPressed = action == GLFW_PRESS;
			}
		}

		virtual void end() {
			spriteShader.reset();
			sun.reset();

			background.reset();
			mo.reset();
			eve.reset();
			walle.reset();

			camera.reset();
			guiText.release();

			rubbishPool.reset();
			blockPool.reset();

			SceneManager::getInstance().removeAllObjects();
		}
};

#endif