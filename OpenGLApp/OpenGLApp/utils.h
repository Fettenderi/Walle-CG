#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <glm/vec3.hpp>
#include <string>
#include <random>

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<float> distf(-1.0f, 1.0f);

static float remap(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static float lerp(float a, float b, float t) {
	return a + t * (b - a);
}

static float explerp(float a, float b, float t) {
    return a + (1.0f - exp(-t)) * (b - a);
}

static glm::vec2 explerpVec2(glm::vec2 a, glm::vec2 b, float t) {
    return glm::vec2(explerp(a.x, b.x, t), explerp(a.y, b.y, t));
}

static float sign(float value) {
    if (value >= 0.0f) return 1.0f;
    return -1.0f;
}

static float clamp(float min, float max, float value) {
	if (value > max) return max;
	if (value < min) return min;
	return value;
}

static glm::vec2 clamp(glm::vec2 min, glm::vec2 max, glm::vec2 value) {
	return glm::vec2(clamp(min.x, max.x, value.x), clamp(min.y, max.y, value.y));
}

static glm::vec3 hex_color(const std::string& hex) {
    std::string s = hex;
    if (s[0] == '#') {
        s.erase(0, 1);
    }

    if (s.size() != 6) {
        throw std::invalid_argument("Formato HEX non valido");
    }

    int r = std::stoi(s.substr(0, 2), nullptr, 16);
    int g = std::stoi(s.substr(2, 2), nullptr, 16);
    int b = std::stoi(s.substr(4, 2), nullptr, 16);

    return glm::vec3(r, g, b) / 255.0f;
}

static float getNextRandom() {
    return distf(gen);
}

static float getNextRandomRange(float min, float max) {
    return remap(getNextRandom(), -1.0f, 1.0f, min, max);
}

#endif