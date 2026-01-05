#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;

// texture samplers
uniform sampler2D mainTexture;
uniform sampler2D normalTexture;

uniform vec3 ambientColor;

uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform float lightStrength;

uniform vec3 sunColor;
uniform vec3 sunPosition;
uniform float sunStrength;

uniform vec3 viewPosition;

void main() {
	// ambience
	float ambientStrength = 0.3;
	vec3 ambient = ambientStrength * ambientColor;
	
	// diffuse
	vec3 norm = normalize(vec3(0.0, 0.0, 1.0));

	vec3 lightDir = normalize(lightPosition - FragPos);
	vec3 sunDir = normalize(sunPosition - FragPos);

	float diffLight = max(dot(norm, lightDir), 0.0f);
	float diffSun = max(dot(norm, sunDir), 0.0f);

	vec3 diffuse = lightStrength * diffLight * lightColor + sunStrength * diffSun * sunColor;

	// specular
	float specularStrength = 0.2;
	vec3 viewDir = normalize(viewPosition - FragPos);

	vec3 reflectLightDir = reflect(-lightDir, norm);
	vec3 reflectSunDir = reflect(-sunDir, norm);

	float specLight = pow(max(dot(viewDir, reflectLightDir), 0.0f), 32);
	float specSun = pow(max(dot(viewDir, reflectSunDir), 0.0f), 32);

	vec3 specular = specularStrength * (lightStrength * specLight * lightColor + sunStrength * specSun * sunColor);
	//vec3 specular = specularStrength * specSun * sunColor;

	vec4 objectColor = texture(mainTexture, TexCoord);
	vec3 result = max((ambient + diffuse + specular) * objectColor.rgb, 0.0);
	// FragColor = vec4(lightDir.zzz, objectColor.a);
	FragColor = vec4(result, objectColor.a);
}