#ifndef STATS_MANAGER_H
#define STATS_MANAGER_H

#include <list>

class StatsManager {
	public:

		static StatsManager& getInstance() {
			static StatsManager instance; // thread-safe da C++11
			return instance;
		}

		int currentRubbish = 0;
		int collectedBlocks = 0;
		int highScore = 0;

		float maxBlockProgress = -1.0f;
	private:
		StatsManager() = default;

		StatsManager(const StatsManager&) = delete;
		StatsManager& operator=(const StatsManager&) = delete;
};

#endif
