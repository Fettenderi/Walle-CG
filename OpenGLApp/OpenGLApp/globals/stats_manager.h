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
		int collectedRubbish = 0;
		int collectedBlocks = 0;
		int time = 0;
		int flashlightBattery = 5;

		bool flashlightBatteryChanging = true;
		bool isEasyMode = false;

		float maxBlockProgress = -0.643f;
	private:
		StatsManager() = default;

		StatsManager(const StatsManager&) = delete;
		StatsManager& operator=(const StatsManager&) = delete;
};

#endif
