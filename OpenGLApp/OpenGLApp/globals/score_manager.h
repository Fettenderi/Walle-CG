#ifndef SCORE_MANAGER_H
#define SCORE_MANAGER_H

#include <string>
#include <fstream>
#include <filesystem>
#include <map>

//#include "../score_history/SimpleJson.h"

#include "../score_history/json.hpp"
using json = nlohmann::json;

class ScoreManager {
	public:

		static ScoreManager& getInstance() {
			static ScoreManager instance; // per singleton
			return instance;
		}

		std::map<std::string, std::string> scores;
		

		void load(){
			if (std::filesystem::exists(filePath)){
				std::ifstream file(filePath);
				json j;
				file >> j;

				for (auto& [key, value] : j.items()){
					//if (value.is_number_integer())
						//scores[key] = value.get<int>();
				}
			}
		}
		void save(){
			json j;

			for (const auto& [key, value] : scores){
				j[key] = value;
			}

			std::ofstream file(filePath);
			file << j.dump(4);
		};

		void set(const std::string& key, std::string value){
			scores[key] = value;
		}

		std::string get(const std::string& key) const{
			auto it = scores.find(key);
			if (it == scores.end())
				return 0;

			return it->second;
		}

	private:
		ScoreManager() = default;

		ScoreManager(const ScoreManager&) = delete;
		ScoreManager& operator=(const ScoreManager&) = delete;

		std::string filePath = "score_history/scores.json";
};

// collectedBlocks or highScore;

#endif
