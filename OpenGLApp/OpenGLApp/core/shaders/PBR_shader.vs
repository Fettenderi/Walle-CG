#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 model;
uniform mat4 camera;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 UV;
out mat3 TBN;

void main() {
	vec3 T = normalize(vec3(model * vec4(aTangent,		0.0)));
	vec3 B = normalize(vec3(model * vec4(aBitangent,	0.0)));
	vec3 N = normalize(vec3(model * vec4(aNormal,		0.0)));

	// used for tangent space - world space mapping
	TBN = mat3(T, B, N);

	FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    UV = aUV;
	gl_Position = projection * camera * model * vec4(aPos, 1.0);
}