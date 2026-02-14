#version 330 core

in vec3 Normal;
in vec3 FragPos;
in vec2 UV;
in mat3 TBN;

out vec4 FragColor;

const float PI = 3.14159265359;

struct Light {
	vec3 position;
	vec3 color;
};

// texture samplers
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_normal1;
uniform sampler2D texture_metalness1;

uniform bool hasNormalMap = false;
uniform bool hasMetalnessMap = false;

// light properties
uniform vec3 ambient;

uniform Light lights[2];

// camera position
uniform vec3 viewPosition;

// pbr functions
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

void main() {
	// diffuse color
	vec3 diffuseColor = pow(texture(texture_diffuse2, vec2(UV.x, 1.0 - UV.y)).rgb, vec3(2.2)); // to linear space
	float diffuseAlpha = texture(texture_diffuse2, vec2(UV.x, 1.0 - UV.y)).a;

	vec3 normal;

	// normal map coordinate mapping
	if (hasNormalMap) {
		normal = texture(texture_normal1, vec2(UV.x, 1.0 - UV.y)).rgb;
		normal = normal * 2.0 - 1.0;
		normal = normalize(TBN * normal);
	} else {
		normal = normalize(Normal);
	}

	// metallic - roughness
	float ambientOcclusion = 1.0;
	float roughness = 0.6;
	float metallic = -1.0;
	vec3 ORM = vec3(ambientOcclusion, roughness, metallic);

	if (hasMetalnessMap) {
		ORM = texture(texture_metalness1, vec2(UV.x, 1.0 - UV.y)).rgb;
		ambientOcclusion = ORM.r;
		roughness = ORM.g * 2.0 - 1.0;
		metallic = ORM.b * 2.0 - 1.0;
	}

	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, diffuseColor, metallic);
	vec3 viewDir = normalize(viewPosition - FragPos);
	
	vec3 result = vec3(0.0);
	for (int i = 0; i < 2; i++) {
		// diffuse lighting
		vec3 lightDirection = normalize(lights[i].position - FragPos);
		vec3 halfway = normalize(viewDir + lightDirection);

		float distance = length(lights[i].position - FragPos);
		float attenuation = 1.0 / (distance * distance);
		//float attenuation = 1.8 * exp(-distance * distance / 10.0);

		vec3 radiance = lights[i].color * attenuation;

		vec3 fresnel = fresnelSchlick(max(dot(halfway, viewDir), 0.0), F0);

		vec3 kS = fresnel;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;

		// cook-torrance specular reflection
		float NDF = DistributionGGX(normal, halfway, roughness);
		float G = GeometrySmith(normal, viewDir, lightDirection, roughness);

		vec3 numerator = NDF * G * fresnel;
		float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDirection), 0.0) + 0.0001;
		vec3 specular = numerator / denominator;

		float diffusedLight = max(dot(normal, lightDirection), 0.0);

		// resulting radiance
		result += (kD * diffuseColor / PI + specular) * radiance * diffusedLight;
	}

	result += ambientOcclusion * ambient * 0.001;
	result = result / (result + vec3(1.0));
	result = pow(result, vec3(1.0/2.2)); // back to sRGB space

	FragColor = vec4(result, diffuseAlpha);
}

//	v			  v			      v
// --- code from learnopnegl.com ---
//	v			  v			      v

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}