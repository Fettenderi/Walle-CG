#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <map>

#include "../score_history/json.hpp"
using json = nlohmann::json;

class FileManager {
	public:
		enum FilePath {
			SCORES, CONFIG
		};

		static FileManager& getInstance() {
			static FileManager instance; // per singleton
			return instance;
		}

		std::map<std::string, std::string> scores;
		std::map<std::string, std::string> config;
		
		void load(FilePath path){
			std::filesystem::path pathString = path == FilePath::SCORES ? scoresPath : configPath;
			std::map<std::string, std::string> *savedMap = path == FilePath::SCORES ? &scores : &config;

			if (!std::filesystem::exists(saveDirectory)) {
				std::filesystem::create_directories(saveDirectory);
			}

			if (std::filesystem::exists(pathString)){
				std::ifstream file(pathString);
				json j;
				file >> j;

				for (auto& [key, value] : j.items()){
					(*savedMap)[key] = value;
				}
			}
		}

		void save(FilePath path){
			json j;

			std::filesystem::path pathString = path == FilePath::SCORES ? scoresPath : configPath;
			std::map<std::string, std::string> *savedMap = path == FilePath::SCORES ? &scores : &config;

			for (const auto& [key, value] : (*savedMap)){
				j[key] = value;
			}

			std::ofstream file(pathString);
			file << j.dump(4);
		};

		void set(FilePath path, const std::string& key, std::string value){
			std::map<std::string, std::string> *savedMap = path == FilePath::SCORES ? &scores : &config;

			(*savedMap)[key] = value;
		}

		std::string get(FilePath path, const std::string& key) {
			std::map<std::string, std::string> *savedMap = path == FilePath::SCORES ? &scores : &config;

			auto it = (*savedMap).find(key);
			if (it == (*savedMap).end())
				return "";

			return it->second;
		}

	private:
		FileManager() {
			saveDirectory = "C:/Wall-E";
			scoresPath = "C:/Wall-E/scores.json";
			configPath = "C:/Wall-E/config.json";
		}

		FileManager(const FileManager&) = delete;
		FileManager& operator=(const FileManager&) = delete;

		std::filesystem::path saveDirectory;

		std::filesystem::path scoresPath;
		std::filesystem::path configPath;

};