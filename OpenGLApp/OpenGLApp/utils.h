#ifndef UTILS_H
#define UTILS_H

#include <cmath>

float lerp(float a, float b, float t) {
	return a + t * (b - a);
}

float explerp(float a, float b, float t) {
	return a + (1.0f - exp(-t)) * (b - a);
}

#endif