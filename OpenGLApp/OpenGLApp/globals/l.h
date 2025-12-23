#ifndef UTILS_H
#define UTILS_H

#include <cmath>

static float lerp(float a, float b, float t) {
	return a + t * (b - a);
}

static float explerp(float a, float b, float t) {
	return a + (1.0f - exp(-t)) * (b - a);
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

#endif