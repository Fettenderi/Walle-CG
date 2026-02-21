#include "scene_manager.h"

#include "../scenes/main_menu_scene.h"
#include "../scenes/game_scene.h"
#include "../scenes/game_over_scene.h"
#include "../scenes/welcome_scene.h"

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

void SceneManager::changeSubscene(std::shared_ptr<Scene> mainScene, SceneID newSceneID, GLFWwindow* windowRef, std::function<void(std::shared_ptr<Scene>)> preInit) {
	std::shared_ptr<Scene> newScene;

	switch (newSceneID) {
	case SceneID::MMWelcomeScene:
		newScene = std::make_shared<WelcomeScene>(windowRef);
		break;
	case SceneID::MMDifficultyScene:
		newScene = std::make_shared<MainMenuScene>(windowRef);
		break;
	case SceneID::MMInstructionsScene:
		newScene = std::make_shared<MainMenuScene>(windowRef);
		break;
	}

	if (mainScene->currentSubscene) {
		mainScene->currentSubscene->end();
	}

	mainScene->currentSubscene = newScene;
	mainScene->currentSubscene->parentScene = mainScene;
	preInit(mainScene->currentSubscene);
	mainScene->currentSubscene->init();
}

