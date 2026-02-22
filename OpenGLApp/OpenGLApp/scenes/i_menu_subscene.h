#pragma once

#include <memory>

#include <irrKlang.h>

#include "../core/shader.h"

class IMenuSubscene {
	public:
		std::shared_ptr<Shader> spriteShader;
		std::shared_ptr<Shader> PBRShader;

		irrklang::ISoundEngine* soundPlayer;

		~IMenuSubscene() {
			spriteShader.reset();
			PBRShader.reset();
		}
};