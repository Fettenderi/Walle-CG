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
uniform vec3 viewPosition;

void main() {
	// ambience
	float ambientStrength = 0.3;
	vec3 ambient = ambientStrength * ambientColor;
	
	// diffuse
	vec3 norm = normalize(vec3(0.0, 0.0, 1.0));
	vec3 lightDir = normalize(lightPosition - FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = diff * lightColor;

	// specular
	float specularStrength = 0.6;
	vec3 viewDir = normalize(viewPosition - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
	vec3 specular = specularStrength * spec * lightColor;

	vec4 objectColor = texture(mainTexture, TexCoord);
	vec3 result = max((ambient + diffuse + specular) * objectColor.rgb, 0.0);
	// FragColor = vec4(lightDir.zzz, objectColor.a);
	FragColor = vec4(result, objectColor.a);
}