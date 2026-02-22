#ifndef SCORE_MANAGER_H
#define SCORE_MANAGER_H

#include <string>
#include <fstream>
#include <filesystem>

//#include "../score_history/SimpleJson.h"

#include "../score_history/json.hpp"
using json = nlohmann::json;

class ScoreManager {
	public:

		static ScoreManager& getInstance() {
			static ScoreManager instance; // per singleton
			return instance;
		}

		int bestScore = 0;
		int currentScore = 0;
		

		void load()
		{
			if (std::filesystem::exists(filePath))
			{
				std::ifstream file(filePath);
				json j;
				file >> j;

				bestScore = j.value("best_score", 0);
			}
			else
			{
				bestScore = 0;
			}
		}
		void save()
		{
			if (currentScore > bestScore)
				bestScore = currentScore;

			json j;
			j["best_score"] = bestScore;
			j["current_score"] = currentScore;

			std::ofstream file(filePath);
			file << j.dump(4); // 4 = indentazione
		};

		void setCurrentScore(int score) {
			currentScore = score;
		};

	private:
		ScoreManager() = default;

		ScoreManager(const ScoreManager&) = delete;
		ScoreManager& operator=(const ScoreManager&) = delete;

		std::string filePath = "score_history/scores.json";
};

//int collectedBlocks = 0;
//int highScore = 0;

#endif
