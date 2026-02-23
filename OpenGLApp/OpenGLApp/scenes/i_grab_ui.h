#pragma once

#include <memory>

#include "../characters/image.h"

class IGrabUI {
public:
	std::shared_ptr<Image> grabbedImage;
	glm::vec2 startingPosition;

	bool hasGrabbedImage = false;
	bool scaleImage = false;

	~IGrabUI() {
		grabbedImage.reset();
	}

	void updateGrabUI(GLFWwindow* window, float deltaTime) {
		if (!hasGrabbedImage) return;

		double xpos, ypos;
		int width, height;

		glfwGetCursorPos(window, &xpos, &ypos);
		glfwGetWindowSize(window, &width, &height);

		float scX = (float)xpos / (float)width * 2.0f - 1.0f;
		float scY = -(float)ypos / (float)height * 2.0f + 1.0f;

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			if (abs(scX) > abs(scY)) grabbedImage->setPosition(startingPosition + glm::vec2(scX, 0.0f));
			else grabbedImage->setPosition(startingPosition + glm::vec2(0.0f, scY));
		}
		else {
			grabbedImage->setPosition(startingPosition + glm::vec2(scX, scY));
		}


		if (!scaleImage) return;

		float velocity = 0.0f;
		if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
			velocity += 1.0f;

		if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
			velocity -= 1.0f;

		if (velocity == 0.0f) return;

		grabbedImage->setUniformScale(grabbedImage->getUniformScale() + velocity * (float)deltaTime * 1.0f);
		printf("grabbedImage: pos:(%f, %f), scale: %f\n", grabbedImage->getPosition().x, grabbedImage->getPosition().y, grabbedImage->getUniformScale());
	}

	bool releaseImage(glm::vec2 cursorPosition, std::function<void()> additionalBehaviour) {
		if (!hasGrabbedImage) return false;

		grabbedImage->setPosition(cursorPosition);
		printf("grabbedImage: pos:(%f, %f), scale: %f\n", grabbedImage->getPosition().x, grabbedImage->getPosition().y, grabbedImage->getUniformScale());
		hasGrabbedImage = false;
		grabbedImage = nullptr;

		additionalBehaviour();
		return true;
	}

	void grabImage(glm::vec2 cursorPosition, std::shared_ptr<Image> image) {
		grabbedImage = image;
		hasGrabbedImage = true;
		startingPosition = grabbedImage->getPosition() - cursorPosition;
	}
};