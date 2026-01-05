#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <list>

#include "../characters/character.h"
#include "../characters/rubbish.h"
#include "../characters/block.h"

#include "../core/scene.h"
#include "../core/camera.h"
#include "../core/light.h"
#include "../core/pool.h"

class SceneManager {
	public:
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

		void addObject(std::shared_ptr<Character> newObj) {
			objects.push_back(newObj);
		}

		void removeObject(std::shared_ptr<Character> obj) {
			objects.remove(obj);
		}

		std::shared_ptr<Scene> currentScene;

		//std::unique_ptr<ObjectPool<Rubbish>> rubbishPool;
		//std::unique_ptr<ObjectPool<Block>> blockPool;

		std::shared_ptr<Camera> camera;

		std::shared_ptr<Light> sun;
		std::shared_ptr<Light> flashlight;

	private:
		SceneManager() = default;

		SceneManager(const SceneManager&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;

		std::list<std::shared_ptr<Character>> objects;
};

#endif
