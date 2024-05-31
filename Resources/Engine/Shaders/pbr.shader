<shader>
	<type name = "includeShader" />

	<source>
	<!--

#ifndef PBR_CODE
#define PBR_CODE

// Most of the pbr util functions are from https://learnopengl.com

const float PI = 3.14159265359;

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// efficient VanDerCorpus calculation.
float RadicalInverse_VdC(uint bits) 
{
		bits = (bits << 16u) | (bits >> 16u);
		bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
		bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
		bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
		bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
		return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
		float a = roughness*roughness;
		float a2 = a*a;
		float NdotH = max(dot(N, H), 0.0);
		float NdotH2 = NdotH*NdotH;

		float nom   = a2;
		float denom = (NdotH2 * (a2 - 1.0) + 1.0);
		denom = PI * denom * denom;

		return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
		float r = (roughness + 1.0);
		float k = (r*r) / 8.0;

		float nom   = NdotV;
		float denom = NdotV * (1.0 - k) + k;

		return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
		float NdotV = max(dot(N, V), 0.0);
		float NdotL = max(dot(N, L), 0.0);
		float ggx2 = GeometrySchlickGGX(NdotV, roughness);
		float ggx1 = GeometrySchlickGGX(NdotL, roughness);

		return ggx1 * ggx2;
}

float GeometrySchlickGGXForIBL(float NdotV, float roughness)
{
		float r = roughness;
		float k = (r*r) / 2.0;

		float nom   = NdotV;
		float denom = NdotV * (1.0 - k) + k;

		return nom / denom;
}

float GeometrySmithForIBL(vec3 N, vec3 V, vec3 L, float roughness)
{
		float NdotV = max(dot(N, V), 0.0);
		float NdotL = max(dot(N, L), 0.0);
		float ggx2 = GeometrySchlickGGXForIBL(NdotV, roughness);
		float ggx1 = GeometrySchlickGGXForIBL(NdotL, roughness);

		return ggx1 * ggx2;
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness*roughness;
			
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
			
	// from spherical coordinates to cartesian coordinates - halfway vector
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;
			
	// from tangent-space H vector to world-space sample vector
	vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
			
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
		return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
		return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 BaseReflectivityPBR(vec3 F0, vec3 albedo, float metallic)
{
	return mix(F0, albedo, metallic);
}

vec3 CookTorranceBRDF(vec3 fragToEye, vec3 normal, vec3 lightDir, vec3 halfway, float roughness, vec3 F0, out vec3 fresnel)
{
	float NDF = DistributionGGX(normal, halfway, roughness);
	float geometry = GeometrySmith(normal, fragToEye, lightDir, roughness);
	fresnel = FresnelSchlick(max(dot(halfway, fragToEye), 0.0), F0);
	vec3 numerator = NDF * geometry * fresnel;
	float denominator = 4.0 * max(dot(normal, fragToEye), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001; // 0.0001 to prevent divide by zero
	vec3 specular = numerator / denominator;

	return specular;
}

vec3 PBR(vec3 fragPos, vec3 normal, vec3 fragToEye, vec3 albedo, float metallic, float roughness, vec3 lightDir, vec3 lightColor)
{
	// Base reflectivity
	vec3 F0 = BaseReflectivityPBR(vec3(0.04), albedo, metallic);

	vec3 halfway = normalize(lightDir + fragToEye);

	// Cook-Torrance BRDF
	vec3 fresnel;
	vec3 specular = CookTorranceBRDF(fragToEye, normal, lightDir, halfway, roughness, F0, fresnel);

	vec3 kS = fresnel; // Specular part
	// Energy conservation
	vec3 kD = vec3(1.0) - kS; // Diffuse part
	// Only non-metals have diffuse part. (Also works fine with metallic values between 0-1)
	kD *= 1.0 - metallic;

	// Note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	// Scale light by light and normal vectors angle
	vec3 outIrradiance = (kD * albedo / PI + specular) * lightColor * max(dot(normal, lightDir), 0.0);

	return outIrradiance;
}

#endif
	-->
	</source>
</shader>