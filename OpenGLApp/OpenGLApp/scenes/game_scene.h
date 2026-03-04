#ifndef GAME_SCENE_H
#define GAME_SCENE_H

#include <string>

#include "../globals/scene_manager.h"
#include "../globals/file_manager.h"

#include "../core/scene.h"
#include "../core/pool.h"
#include "../core/timer.h"
#include "../core/text.h"

#include "../characters/image.h"
#include "../characters/character.h"
#include "../characters/walle.h"
#include "../characters/eve.h"
#include "../characters/mo.h"
#include "../characters/rubbish.h"
#include "../characters/block.h"

class GameScene : public Scene {
	public:
		GameScene(GLFWwindow* windowRef) : Scene(windowRef) {};

		virtual void init() {
			glDisable(GL_DEPTH_TEST);

			// loading config
			FileManager::getInstance().load(FileManager::CONFIG);
			
			maxRubbish = to_int(FileManager::getInstance().get(FileManager::CONFIG, "glb_max_rubbish"));
			if (maxRubbish == 0) {
				maxRubbish = 10;
				FileManager::getInstance().set(FileManager::CONFIG, "glb_max_rubbish", std::to_string(maxRubbish));
			}

			totalTime = to_float(FileManager::getInstance().get(FileManager::CONFIG, "glb_day_night_duration"));
			if (totalTime == 0.0f) {
				totalTime = 500.0f;
				FileManager::getInstance().set(FileManager::CONFIG, "glb_day_night_duration", std::to_string(totalTime));
			}

			nightPercentage = to_float(FileManager::getInstance().get(FileManager::CONFIG, "glb_night_percentage"));
			if (nightPercentage == 0.0f) {
				nightPercentage = 0.3f;
				FileManager::getInstance().set(FileManager::CONFIG, "glb_night_percentage", std::to_string(nightPercentage));
			}

			float startingTime = to_float(FileManager::getInstance().get(FileManager::CONFIG, "glb_starting_time"));
			FileManager::getInstance().set(FileManager::CONFIG, "glb_starting_time", std::to_string(startingTime));

			currentDayNightFrequency = (float)PI / (totalTime * (1.0f - nightPercentage));
			dayNightElapsed = startingTime;

			StatsManager::getInstance().isEasyMode = to_bool(FileManager::getInstance().get(FileManager::CONFIG, "glb_easy_mode"));
			// ridondante?
			FileManager::getInstance().set(FileManager::CONFIG, "glb_easy_mode", StatsManager::getInstance().isEasyMode ? "true" : "false");

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
			GUIShader = std::make_shared<Shader>("core/shaders/GUI_shader.vs", "core/shaders/GUI_shader.fs");

			// pools initialization
			rubbishPool = std::make_shared<ObjectPool<Rubbish>>(10, spriteShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.4f, 0.32f));
			blockPool = std::make_shared<ObjectPool<Block>>(30, spriteShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.25f, 0.25f));

			// images
			debugImage = std::make_shared<Image>(spriteShader, "assets/textures/block.png", glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));

			background = std::make_shared<Image>(spriteShader, "assets/textures/bg_placeholder.png", glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), 2.0f);
			
			guiBase = std::make_shared<Image>(GUIShader, "assets/textures/ui/gui_base.png", glm::vec2(0.0f, 0.0f), glm::vec2(0.894f, 0.151f), 2.2f);
			dayNightIndicator = std::make_shared<Image>(GUIShader, "assets/textures/ui/daytime_atlas.png", glm::vec2(0.0f, 0.0f), glm::vec2(0.894f, 0.151f), 2.2f);
			compressorIndicator = std::make_shared<Image>(GUIShader, "assets/textures/ui/compressor_atlas.png", glm::vec2(0.0f, 0.0f), glm::vec2(0.894f, 0.151f), 2.2f);
			batteryIndicator = std::make_shared<Image>(GUIShader, "assets/textures/ui/stamina_atlas.png", glm::vec2(0.0f, 0.0f), glm::vec2(0.894f, 0.151f), 2.2f);
			scoreIndicator = std::make_shared<Image>(GUIShader, "assets/textures/ui/score_indicator.png", glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), 0.391663f);

			dayNightIndicator->setAtlasGrid(1, 5);
			compressorIndicator->setAtlasGrid(1, 5);
			batteryIndicator->setAtlasGrid(1, 11);

			guiOffset = glm::vec2(0.0f, -0.811f);

			// characters
			float temp = to_float(FileManager::getInstance().get(FileManager::CONFIG, "ch_eve_speed"));
			if (temp == 0.0f) {
				temp = 1.3f;
				FileManager::getInstance().set(FileManager::CONFIG, "ch_eve_speed", std::to_string(temp));
			}

			eve = std::make_shared<Eve>(rubbishPool, spriteShader, glm::vec2(2.0f, 2.0f), glm::vec2(0.28f, 0.4f), temp);
			
			temp = to_float(FileManager::getInstance().get(FileManager::CONFIG, "ch_walle_speed"));
			if (temp == 0.0f) {
				temp = 1.0f;
				FileManager::getInstance().set(FileManager::CONFIG, "ch_walle_speed", std::to_string(temp));
			}

			walle = std::make_shared<Walle>(rubbishPool, blockPool, spriteShader, glm::vec2(0.0f, 0.3f), glm::vec2(0.4f, 0.4f), 0.0f, temp);
			
			if (!StatsManager::getInstance().isEasyMode) {
				temp = to_float(FileManager::getInstance().get(FileManager::CONFIG, "ch_mo_speed"));
				if (temp == 0.0f) {
					temp = 1.0f;
					FileManager::getInstance().set(FileManager::CONFIG, "ch_mo_speed", std::to_string(temp));
				}

				mo = make_shared<Mo>(blockPool, spriteShader, glm::vec2(0.0f, -0.3f), glm::vec2(0.28f, 0.4f), 0.0f, temp);
				
				SceneManager::getInstance().addObject(mo);
			}

			SceneManager::getInstance().addObject(walle);
			SceneManager::getInstance().addObject(eve);

			// lights setup
			sun = std::make_shared<Light>(glm::vec3(1.0f, 0.0f, 0.0f), 80.0f, "#ffffab");
			SceneManager::getInstance().sun = sun;
			SceneManager::getInstance().ambientStrength = 0.3f;

			GUIShader->use();
			spriteShader->setInt("mainTexture", 0);
			spriteShader->setMat4("camera", camera->getViewMatrix());

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

			// debug
			changeDebugParameters();
			changeDebug1Parameters();

			for (std::shared_ptr<Character> obj : SceneManager::getInstance()) {
				obj->yDebug = 1.96f + 2.33f;
			}

			// global update
			if (StatsManager::getInstance().currentRubbish > maxRubbish) {
				StatsManager::getInstance().currentRubbish = 0;
				StatsManager::getInstance().time = (int)ceil(elapsed);
				SceneManager::getInstance().changeScene(SceneManager::SceneID::GameOverScene, window);
				return (float)deltaTime;
			}
			
			camera->update((float)deltaTime);

			background->setPosition(camera->getPosition2D());
			background->renderSprite();

			/*
			debugImage->setPosition(glm::vec2(0.0f, -0.643f));
			debugImage->setUniformScale(debug1);
			debugImage->renderSprite();
			*/

			handleDayNightCycle();

			GUIShader->use();
			GUIShader->setMat4("camera", camera->getViewMatrix());

			spriteShader->use();
			spriteShader->setMat4("camera", camera->getViewMatrix());
			spriteShader->setVec3("lights[0].position", sun->position);
			spriteShader->setVec3("lights[0].color", sun->getColor() * sun->strength);
			spriteShader->setVec3("viewPosition", camera->getPosition());

			// mo movement
			if (!StatsManager::getInstance().isEasyMode && lmbPressed) {
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

			compressorIndicator->setTile(0, StatsManager::getInstance().collectedRubbish);

			int chargeIndicator = 0;
			if (StatsManager::getInstance().flashlightBatteryChanging) {
				chargingElapsed += deltaTime;

				if (previousEnergy != StatsManager::getInstance().flashlightBattery) {
					chargingElapsed = 0.0f;
					previousEnergy = StatsManager::getInstance().flashlightBattery;
				}

				chargeIndicator = round((sin(chargingElapsed * 3.0f - PI / 2.0f) + (isNight ? -1.0f : 1.0f)) / 2.0f);
			}
			else {
				chargingElapsed = 0.0f;
			}


			batteryIndicator->setTile(0, 10 - (int)clamp(0.0f, 10.0f, StatsManager::getInstance().flashlightBattery + chargeIndicator));

			guiBase->setPosition(camera->getPosition2D() + guiOffset);
			dayNightIndicator->setPosition(camera->getPosition2D() + guiOffset);
			compressorIndicator->setPosition(camera->getPosition2D() + guiOffset);
			batteryIndicator->setPosition(camera->getPosition2D() + guiOffset);
			scoreIndicator->setPosition(camera->getPosition2D() + glm::vec2(-0.828052f, 0.820165f));

			guiBase->renderSprite();
			dayNightIndicator->renderSprite();
			compressorIndicator->renderSprite();
			batteryIndicator->renderSprite();
			scoreIndicator->renderSprite();

			guiText->RenderText(std::format("{:02.0f}:{:02.0f}", floor((float)(elapsed) / 60.0f), mod((float)elapsed, 60.0f)), glm::vec2(width / 2.0f - 34.0f, height - 40.0f), 0.7f, "#0a1518");
			guiText->RenderText(std::format(": {}", StatsManager::getInstance().collectedBlocks), toScreenSpace(glm::vec2(-0.635969f, 0.785183f), width, height), 0.7f, "#0a1518");
		}

		glm::vec2 toScreenSpace(glm::vec2 position, int width, int height) {
			float x = (0.5f * position.x + 0.5f) * width;
			float y = (0.5f * position.y + 0.5f) * height;

			return glm::vec2(x, y);
		}


		virtual void mouseCallback(GLFWwindow* passedWindow, int button, int action, int mods) {
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				lmbPressed = action == GLFW_PRESS;
			}
		}

		virtual void end() {
			GUIShader.reset();
			spriteShader.reset();
			sun.reset();

			background.reset();

			guiBase.reset();
			dayNightIndicator.reset();
			compressorIndicator.reset();
			batteryIndicator.reset();
			scoreIndicator.reset();

			mo.reset();
			eve.reset();
			walle.reset();

			camera.reset();
			guiText.release();

			rubbishPool.reset();
			blockPool.reset();

			SceneManager::getInstance().removeAllObjects();
		}

	private:
		std::shared_ptr<Shader> spriteShader;
		std::shared_ptr<Shader> GUIShader;
		std::shared_ptr<Light> sun;

		std::shared_ptr<Image> debugImage;
		std::shared_ptr<Image> background;
		std::shared_ptr<Image> guiBase;
		std::shared_ptr<Image> dayNightIndicator;
		std::shared_ptr<Image> compressorIndicator;
		std::shared_ptr<Image> batteryIndicator;
		std::shared_ptr<Image> scoreIndicator;

		glm::vec2 guiOffset;

		std::shared_ptr<Mo> mo;
		std::shared_ptr<Eve> eve;
		std::shared_ptr<Walle> walle;

		std::shared_ptr<Camera> camera;
		std::unique_ptr<Text> guiText;

		std::shared_ptr<ObjectPool<Rubbish>> rubbishPool;
		std::shared_ptr<ObjectPool<Block>> blockPool;

		glm::vec3 bgColor = glm::vec3(0.6f, 0.42f, 0.33f);

		glm::vec2 debug = glm::vec2(0.0f, 0.0f);

		float totalTime;
		float nightPercentage;
		float currentDayNightFrequency = 0.0f;
		float dayNightPhase = 0.0f;
		float dayNightElapsed = 0.0f;
		float chargingElapsed = 0.0f;
		int previousEnergy = 5;

		bool lmbPressed = false;
		bool isNight = false;

		int maxRubbish;

		void handleDayNightCycle() {
			dayNightElapsed += deltaTime;

			float newDayNightFrequency;

			float time = dayNightElapsed / totalTime - floor(dayNightElapsed / totalTime);

			dayNightIndicator->setTile(0, 4 - (int)floor(time * 5.0f - 0.5f));

			if (time <= (1.0f - nightPercentage)) {
				// day
				newDayNightFrequency = (float)PI / (totalTime * (1.0f - nightPercentage));
				walle->setFlashlight(false);
				eve->setNightMode(false);
				isNight = false;

			}
			else {
				// night
				newDayNightFrequency = (float)PI / (totalTime * nightPercentage);
				walle->setFlashlight(true);
				eve->setNightMode(true);

				isNight = true;
			}

			// When changing frequency you need to add a phase in order to allign 
			// to the next valid y value of the new trig function
			if (currentDayNightFrequency != newDayNightFrequency) {
				dayNightPhase = (currentDayNightFrequency - newDayNightFrequency) * (float)dayNightElapsed + dayNightPhase;
				currentDayNightFrequency = newDayNightFrequency;
			}

			float x = (dayNightElapsed) * newDayNightFrequency + dayNightPhase;

			sun->position = glm::vec3(7.0f * glm::vec4(-cos(x), 0.0f, sin(x), 1.0f) * glm::rotate(glm::mat4(1.0f), glm::radians(11.0f), glm::vec3(1.0f, 0.0f, 0.0f)));

			//printf("%f, %f, %f, %f\n", dayNightElapsed, time, x, dayNightPhase);
		}

		void changeDebugParameters() {
			float velocity = 0.0f;

			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
				velocity += 1.0f;

			if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
				velocity -= 1.0f;

			if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
				velocity *= 2.0f;

			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
				velocity /= 2.0f;


			if (velocity == 0.0f) return;

			debug.y += velocity * (float)deltaTime * 1.0f;
			printf("debug: (%f, %f)\n", debug.x, debug.y);

		}

		void changeDebug1Parameters() {
			float velocity = 0.0f;

			if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
				velocity += 1.0f;

			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
				velocity -= 1.0f;

			if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
				velocity *= 2.0f;

			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
				velocity /= 2.0f;


			if (velocity == 0.0f) return;

			debug.x += velocity * (float)deltaTime * 1.0f;
			printf("debug: (%f, %f)\n", debug.x, debug.y);
		}

};

#endif