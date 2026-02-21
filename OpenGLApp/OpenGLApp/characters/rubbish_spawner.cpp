#include "rubbish_spawner.h"

#include "rubbish.h"
#include "../globals/stats_manager.h"

std::shared_ptr<Character> RubbishSpawner::spawnRubbish(glm::vec2 position, bool isPickable) {
    std::shared_ptr<Rubbish> tempRubbish;// = rubbishPool->getInstance();
    if (tempRubbish == nullptr) return nullptr;

    tempRubbish->show();
    tempRubbish->setPosition(position);
    tempRubbish->isPickable = isPickable;
    tempRubbish->trashAmount = getNextRandomIntRange(2, JUNK_TO_BLOCK / 3 + 1);

    SceneManager::getInstance().addObject(tempRubbish);

    return tempRubbish;
}