#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;

struct Light {
	vec3 position;
	vec3 color;
};

uniform int hTiles = 1;
uniform int tile = 0;

uniform sampler2D mainTexture;

uniform vec3 ambient;

uniform Light lights[2];

uniform vec3 viewPosition;

float specularStrength = 0.2;

void main() {
	vec3 viewDir = normalize(viewPosition - FragPos);

	vec4 objectColor = texture(mainTexture, (TexCoord + vec2(tile, 0.0))/ vec2(hTiles, 1.0));
	
	vec3 norm = normalize(vec3(0.0, 0.0, 1.0));

	vec3 result = vec3(0.0);
	for (int i = 0; i < 2; i++) {
		vec3 lightDir = normalize(lights[i].position - FragPos);
		float distance = length(lights[i].position - FragPos);
		//float attenuation = 1.0 / (distance * distance);
		float attenuation = 1.8 * exp(-distance * distance / 10.0);

		float diffLight = max(dot(norm, lightDir), 0.0);

		vec3 reflectLightDir = reflect(-lightDir, norm);

		float specLight = pow(max(dot(viewDir, reflectLightDir), 0.0), 32);

		result += attenuation * lights[i].color * diffLight + specularStrength * specLight;
	}

	result = max((ambient + result) * objectColor.rgb, 0.0);

	FragColor = vec4(result, objectColor.a);
}