#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform vec2 visibility = vec2(1.0, 1.0);
uniform vec2 tilesConfig = vec2(1.0, 1.0); // x = colonne (hTiles), y = righe (vTiles)
uniform vec2 currentTile = vec2(0.0, 0.0); // x = colonna attuale, y = riga attuale

uniform sampler2D mainTexture;

void main() {
	if (TexCoord.y > visibility.y || TexCoord.x > visibility.x) {
		FragColor = vec4(0.0);
		return;
	}

	FragColor = texture(mainTexture, (TexCoord + currentTile) / tilesConfig);
	//FragColor = vec4(texture(mainTexture, (TexCoord + currentTile) / tilesConfig).rgb, 0.2);
}