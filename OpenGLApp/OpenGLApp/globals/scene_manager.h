#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <list>

#include <irrKlang.h>

#include "../characters/character.h"
#include "../characters/rubbish.h"
#include "../characters/block.h"

#include "../core/scene.h"
#include "../core/camera.h"
#include "../core/light.h"
#include "../core/pool.h"

class SceneManager {
	public:
		enum class SceneID {
			MainMenuScene, GameScene, GameOverScene
		};

		using iterator = std::list<std::shared_ptr<Character>>::iterator;
		using const_iterator = std::list<std::shared_ptr<Character>>::const_iterator;

		iterator begin() { return objects.begin(); }
		iterator end() { return objects.end(); }

		const_iterator begin() const { return objects.begin(); }
		const_iterator end() const { return objects.end(); }

		static SceneManager& getInstance() {
			static SceneManager instance; // thread-safe da C++11
			return instance;
		}

		void ySortObjects() {
			objects.sort([](const std::shared_ptr<Character>& a, const std::shared_ptr<Character>& b) {
					return a->getY() > b->getY();
				});
			inVecCalculated = false;
		}

		void addObject(std::shared_ptr<Character> newObj) {
			objects.push_back(newObj);
		}

		void removeObject(std::shared_ptr<Character> obj) {
			objects.remove(obj);
		}

		std::vector<std::shared_ptr<Character>> getObjects() {
			if (!inVecCalculated) {
				objectsInVec = std::vector<std::shared_ptr<Character>>(objects.begin(), objects.end());
				inVecCalculated = true;
			}

			return objectsInVec;
		}

		void removeAllObjects() {
			for (std::shared_ptr<Character> object : objects) {
				object.reset();
			}

			objects.clear();
		}

		void changeScene(SceneID newSceneID, GLFWwindow* windowRef);

		std::shared_ptr<Scene> currentScene;

		std::shared_ptr<Camera> camera;

		std::shared_ptr<Light> sun;
		std::shared_ptr<Light> flashlight;

		irrklang::ISoundEngine* soundManager;
		irrklang::ISound* inBetweenSound;

	private:
		SceneManager() {
			// IrrKlang
			soundManager = irrklang::createIrrKlangDevice();

			if (soundManager == NULL) {
				printf("Unable to start sound manager");
			}
		};

		SceneManager(const SceneManager&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;

		std::list<std::shared_ptr<Character>> objects;
		std::vector<std::shared_ptr<Character>> objectsInVec;
		bool inVecCalculated = false;
};

#endif
