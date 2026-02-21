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

	// SunLight
	vec3 lightDir = normalize(lights[0].position - FragPos);
	float distance = length(lights[0].position - FragPos);
	//float attenuation = 1.0 / (distance * distance);
	float attenuation = 1.8 * exp(-distance * distance / 10.0);

	float diffLight = max(dot(norm, lightDir), 0.0);

	vec3 reflectLightDir = reflect(-lightDir, norm);

	float specLight = pow(max(dot(viewDir, reflectLightDir), 0.0), 32);

	result += attenuation * lights[0].color * diffLight + specularStrength * specLight;

	// Flashlight
	lightDir = normalize(lights[1].position - FragPos);
	distance = length(lights[1].position - FragPos);
	//attenuation = 1.0 / (distance * distance);
	attenuation = min(3.0f * exp(-distance * distance * 10.0), 1.0);

	diffLight = max(dot(norm, lightDir), 0.0);

	reflectLightDir = reflect(-lightDir, norm);

	specLight = pow(max(dot(viewDir, reflectLightDir), 0.0), 32);

	result += attenuation * lights[1].color * diffLight + specularStrength * specLight;

	// Result
	result += ambient;

	result = max(result * objectColor.rgb, 0.0);

	FragColor = vec4(result, objectColor.a);
}