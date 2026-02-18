#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform float vPercentage = 1.0;
uniform int hTiles = 1;
uniform int tile = 0;

uniform sampler2D mainTexture;

void main() {
	if (TexCoord.y > vPercentage) {
		FragColor = vec4(0.0);
		return;
	}

	FragColor = texture(mainTexture, (TexCoord + vec2(tile, 0.0))/ vec2(hTiles, 1.0));
}