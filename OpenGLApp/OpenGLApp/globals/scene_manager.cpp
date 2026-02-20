#include "scene_manager.h"

#include "../scenes/main_menu_scene.h"
#include "../scenes/game_scene.h"
#include "../scenes/game_over_scene.h"

void SceneManager::changeScene(SceneID newSceneID, GLFWwindow* windowRef) {
	std::shared_ptr<Scene> newScene;

	switch (newSceneID) {
		case SceneID::MainMenuScene:
			newScene = std::make_shared<MainMenuScene>(windowRef);
			break;

		case SceneID::GameScene:
			newScene = std::make_shared<GameScene>(windowRef);
			break;

		case SceneID::GameOverScene:
			newScene = std::make_shared<GameOverScene>(windowRef);
			break;
	}
	
	if (currentScene) {
		currentScene->end();
	}

	currentScene = newScene;
	currentScene->init();
}
