#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;

struct Light {
	vec3 position;
	vec3 color;
};

// texture samplers
uniform sampler2D mainTexture;
uniform sampler2D normalTexture;

uniform vec3 ambient;

uniform Light lights[2];

uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform float lightStrength;

uniform vec3 sunColor;
uniform vec3 sunPosition;
uniform float sunStrength;

uniform vec3 viewPosition;

float specularStrength = 0.2;

void main() {
	vec3 viewDir = normalize(viewPosition - FragPos);

	vec4 objectColor = texture(mainTexture, TexCoord);
	
	vec3 norm = normalize(vec3(0.0, 0.0, 1.0));

	vec3 result = vec3(0.0);
	for (int i = 0; i < 2; i++) {
		vec3 lightDir = normalize(lights[i].position - FragPos);
		//float distance = length(lights[i].position - FragPos);
		//float attenuation = 1.0 / (distance * distance);
		//float attenuation = 1.8 * exp(-distance * distance / 10.0);

		float diffLight = max(dot(norm, lightDir), 0.0f);

		vec3 reflectLightDir = reflect(-lightDir, norm);

		float specLight = pow(max(dot(viewDir, reflectLightDir), 0.0f), 32);

		result += lights[i].color * diffLight + specularStrength * specLight;
	}

	result = max((ambient + result) * objectColor.rgb, 0.0);
	//vec3 sunDir = normalize(sunPosition - FragPos);

	//float diffSun = max(dot(norm, sunDir), 0.0f);

	//vec3 diffuse = lightStrength * diffLight * lightColor + attenuation * sunStrength * diffSun * sunColor;

	// specular

	//vec3 reflectSunDir = reflect(-sunDir, norm);

	//float specSun = pow(max(dot(viewDir, reflectSunDir), 0.0f), 32);

	//vec3 specular = specularStrength * (lightStrength * specLight * lightColor + sunStrength * specSun * sunColor);
	//vec3 specular = specularStrength * specSun * sunColor;

	//vec3 result = max((ambient + diffuse + specular) * objectColor.rgb, 0.0);
	// FragColor = vec4(lightDir.zzz, objectColor.a);

	FragColor = vec4(result, objectColor.a);
}