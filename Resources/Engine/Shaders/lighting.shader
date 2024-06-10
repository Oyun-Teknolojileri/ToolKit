<shader>
	<type name = "includeShader" />
	<include name = "textureUtil.shader" />
	<include name = "shadow.shader" />
	<include name = "lightDataTextureUtils.shader" />
	<include name = "pbr.shader" />
	<uniform name = "shadowDistance" />
	<uniform name = "activeCount" />
	<uniform name = "shadowAtlasSize" />
	<source>
	<!--

#ifndef LIGHTING_SHADER
#define LIGHTING_SHADER

#define MAX_CASCADE_COUNT 4
#define MAX_LIGHT_COUNT 64

// TODO Minimize and pack this data as much as possible
struct _LightData
{
	vec3 pos;
	int type; // Type 1 : Directional light 2 : Point light 3 : Spot light

	vec3 dir;
	float intensity;

	vec3 color;
	float radius;

	float outAngle;
	float innAngle;
	float shadowMapCameraFar;
	int numOfCascades;

	mat4 projectionViewMatrices[MAX_CASCADE_COUNT];
	
	float BleedingReduction;
	float shadowBias;
	int castShadow;
	int softShadows;

	// Shadow filter
	int PCFSamples;
	float PCFRadius;
	float shadowAtlasResRatio; // shadow map resolution / shadow atlas resolution.

	float shadowAtlasLayer[6];
	vec2 shadowAtlasCoord[6]; // Between 0 and 1
};

layout (std140) uniform LightDataBuffer // slot 0
{
	vec4 cascadeDistances; // Max cascade is 4, so this fits.
	_LightData LightData[MAX_LIGHT_COUNT];
};

layout (std140) uniform ActiveLightIndicesBuffer // slot 1
{
	ivec4 activeLightIndices[MAX_LIGHT_COUNT / 4]; // Each component of a vector is a light index.
};

uniform float shadowDistance;
uniform int activeCount;
uniform sampler2DArray s_texture8; // Shadow atlas

/// Deferred rendering uniforms
uniform sampler2D s_texture13; // Light data

uniform float lightDataTextureWidth;
uniform vec2 shadowDirLightsInterval;
uniform vec2 shadowPointLightsInterval;
uniform vec2 shadowSpotLightsInterval;
uniform vec2 nonShadowDirLightsInterval;
uniform vec2 nonShadowPointLightsInterval;
uniform vec2 nonShadowSpotLightsInterval;
uniform float dirShadowLightDataSize;
uniform float pointShadowLightDataSize;
uniform float spotShadowLightDataSize;
uniform float dirNonShadowLightDataSize;
uniform float pointNonShadowLightDataSize;
uniform float spotNonShadowLightDataSize;
uniform float shadowAtlasSize;

const float shadowFadeOutDistanceNorm = 0.9;

bool EpsilonEqual(float a, float b, float eps)
{
	return abs(a - b) < eps;
}

float CalculateDirectionalShadow
(
	vec3 pos, 
	vec3 viewCamPos, 
	mat4 lightProjView, 
	vec2 shadowAtlasCoord, // Shadow map start location in the layer in uv space.
	float shadowAtlasResRatio, // shadowAtlasResRatio = shadow map resolution / shadow atlas resolution.
	float shadowAtlasLayer, 
	int PCFSamples, 
	float PCFRadius, 
	float lightBleedReduction, 
	float shadowBias
)
{
	vec4 fragPosForLight = lightProjView * vec4(pos, 1.0);
	vec3 projCoord = fragPosForLight.xyz;
	projCoord = projCoord * 0.5 + 0.5;

	// Get depth of the current fragment according to lights view
	float currFragDepth = projCoord.z;

	vec2 startCoord = shadowAtlasCoord; // Start coordinate of he shadow map in the atlas.
	vec2 endCoord = shadowAtlasCoord + shadowAtlasResRatio; // End coordinate of the shadow map in the atlas.

	// Find the sample's location in the shadow atlas.
	
	// To do so, we scale the coordinates by the proportion of the shadow map in the atlas. "shadowAtlasResRatio * projCoord.xy"
	// and then offset the scaled coordinate to the beginning of the shadow map via "startCoord + shadowAtlasResRatio * projCoord.xy"
	// which gives us the final uv coordinates in xy and the index of the layer in z
	vec2 uvInAtlas = startCoord + shadowAtlasResRatio * projCoord.xy;
	vec3 sampleCoord = vec3(uvInAtlas, shadowAtlasLayer);

	float shadow = 1.0;

	// Make sure there is always 1 sample.
	// This will offset the shadow randomly in subpixel boundary which will trigger bilinear sampling.
	// Which will yield smooth results even with 1 fetch.
	PCFSamples = max(1, PCFSamples);

	shadow = PCFFilterShadow2D
	(
		s_texture8,
		sampleCoord,
		startCoord,
		endCoord,
		PCFSamples,
		PCFRadius / shadowAtlasSize, // Convert radius in pixel units to uv.
		projCoord.z,
		lightBleedReduction,
		shadowBias
	);

	// Fade shadow out after min shadow fade out distance
	vec3 camToPos = pos - viewCamPos;
	float fade = (length(camToPos) - (shadowDistance * shadowFadeOutDistanceNorm)) / (shadowDistance * (1.0 - shadowFadeOutDistanceNorm));
	fade = clamp(fade, 0.0, 1.0);
	fade = fade * fade;
	return clamp(shadow + fade, 0.0, 1.0);
}

float CalculateSpotShadow
(
	vec3 pos, 
	vec3 lightPos, 
	mat4 lightProjView, 
	float shadowCameraFar, 
	vec2 shadowAtlasCoord,
	float shadowAtlasResRatio, 
	float shadowAtlasLayer, 
	int PCFSamples, 
	float PCFRadius, 
	float lightBleedReduction, 
	float shadowBias
)
{
	vec4 fragPosForLight = lightProjView * vec4(pos, 1.0);
	vec3 projCoord = fragPosForLight.xyz / fragPosForLight.w;
	projCoord = projCoord * 0.5 + 0.5;

	vec3 lightToFrag = pos - lightPos;
	float currFragDepth = length(lightToFrag) / shadowCameraFar;

	vec2 startCoord = shadowAtlasCoord;
	vec3 coord = vec3(startCoord + shadowAtlasResRatio * projCoord.xy, shadowAtlasLayer);

	PCFSamples = max(1, PCFSamples);
	return PCFFilterShadow2D
	(
		s_texture8,
		coord,
		startCoord,
		startCoord + shadowAtlasResRatio,
		PCFSamples,
		PCFRadius / shadowAtlasSize, // Convert radius in pixel units to uv.
		currFragDepth,
		lightBleedReduction,
		shadowBias
	);
}

float CalculatePointShadow
(
	vec3 pos, 
	vec3 lightPos, 
	float shadowCameraFar, 
	vec2 shadowAtlasCoord[6], 
	float shadowAtlasResRatio,
	float shadowAtlasLayer[6], 
	int PCFSamples, 
	float PCFRadius, 
	float lightBleedReduction, 
	float shadowBias
)
{
	vec3 lightToFrag = pos - lightPos;
	float currFragDepth = length(lightToFrag) / shadowCameraFar;

	PCFSamples = max(1, PCFSamples);
	return PCFFilterOmni
	(
		s_texture8,
		shadowAtlasCoord,
		shadowAtlasResRatio,
		shadowAtlasLayer,
		lightToFrag,
		PCFSamples,
		PCFRadius / shadowAtlasSize, // Convert radius in pixel units to uv.
		currFragDepth,
		lightBleedReduction,
		shadowBias
	);
}

// Returns 0 or 1
float RadiusCheck(float radius, float distance)
{
	float radiusCheck = clamp(radius - distance, 0.0, 1.0);
	radiusCheck = ceil(radiusCheck);
	return radiusCheck;
}

float Attenuation(float distance, float radius, float constant, float linear, float quadratic)
{
	float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));
	// Decrase attenuation heavily near radius
	attenuation *= 1.0 - smoothstep(0.0, radius, distance);
	return attenuation;
}

vec3 PBRLighting
(
	vec3 fragPos,				// World space fragment position.
	float viewPosDepth, // View space depth. ( Fragment's distance to camera )
	vec3 normal,				// Fragment's normal in world space. ( can be the geometry normal, or normal map normal )
	vec3 fragToEye,			// Normalized vector from fragment position to camera position in world space normalize( campos - fragPos )
	vec3 viewCamPos,		// Camera position in world space.
	vec3 albedo,
	float metallic,
	float roughness
)
{
	vec3 irradiance = vec3(0.0);

	for (int ii = 0; ii < activeCount; ii++)
	{
		int div = ii / 4;
		int rem = ii % 4;
		int i = activeLightIndices[div][rem];

		// TODO we can create uniform buffer for each light type and iterate those lights in order to avoid this if-else block.
		if (LightData[i].type == 2) // Point light
		{
			// radius check and attenuation
			float lightDistance = length(LightData[i].pos - fragPos);
			float radiusCheck = RadiusCheck(LightData[i].radius, lightDistance);
			float attenuation = Attenuation(lightDistance, LightData[i].radius, 1.0, 0.09, 0.032);

			// lighting
			vec3 lightDir = normalize(LightData[i].pos - fragPos);
			vec3 Lo = PBR(fragPos, normal, fragToEye, albedo, metallic, roughness, lightDir, LightData[i].color * LightData[i].intensity);

			// shadow
			float shadow = 1.0;
			if (LightData[i].castShadow == 1)
			{
				shadow = CalculatePointShadow
				(
					fragPos, 
					LightData[i].pos, 
					LightData[i].shadowMapCameraFar, 
					LightData[i].shadowAtlasCoord, 
					LightData[i].shadowAtlasResRatio,
					LightData[i].shadowAtlasLayer, 
					LightData[i].PCFSamples, 
					LightData[i].PCFRadius, 
					LightData[i].BleedingReduction, 
					LightData[i].shadowBias
				);
			}

			irradiance += Lo * shadow * attenuation * radiusCheck;
		}
		else if (LightData[i].type == 1) // Directional light
		{
			// lighting
			vec3 lightDir = normalize(-LightData[i].dir);
			vec3 Lo = PBR(fragPos, normal, fragToEye, albedo, metallic, roughness, lightDir, LightData[i].color * LightData[i].intensity);

			// shadow
			float depth = abs(viewPosDepth);
			float shadow = 1.0;

			if (LightData[i].castShadow == 1)
			{
				int cascadeOfThisPixel = 0;

				// Cascade selection by depth range.
				for (int ci = 0; ci < LightData[i].numOfCascades; ci++)
				{
					if (depth < cascadeDistances[ci])
					{
						cascadeOfThisPixel = ci;
						break;
					}
				}

				shadow = CalculateDirectionalShadow
				(
					fragPos, 
					viewCamPos, 
					LightData[i].projectionViewMatrices[cascadeOfThisPixel], 
					LightData[i].shadowAtlasCoord[cascadeOfThisPixel].xy / shadowAtlasSize,
					LightData[i].shadowAtlasResRatio,	
					LightData[i].shadowAtlasLayer[cascadeOfThisPixel].x, 
					LightData[i].PCFSamples, 
					LightData[i].PCFRadius,
					LightData[i].BleedingReduction,	
					LightData[i].shadowBias
				);
			}

			irradiance += Lo * shadow;
		}
		else // if (LightData[i].type == 3) Spot light
		{
			// radius check and attenuation
			vec3 fragToLight = LightData[i].pos - fragPos;
			float lightDistance = length(fragToLight);
			float radiusCheck = RadiusCheck(LightData[i].radius, lightDistance);
			float attenuation = Attenuation(lightDistance, LightData[i].radius, 1.0, 0.09, 0.032);

			// Lighting angle and falloff
			float theta = dot(-normalize(fragToLight), LightData[i].dir);
			float epsilon = LightData[i].innAngle - LightData[i].outAngle;
			float intensity = clamp((theta - LightData[i].outAngle) / epsilon, 0.0, 1.0);

			// lighting
			vec3 lightDir = normalize(-LightData[i].dir);
			vec3 Lo = PBR(fragPos, normal, fragToEye, albedo, metallic, roughness, lightDir, LightData[i].color * LightData[i].intensity);

			// shadow
			float shadow = 1.0;
			if (LightData[i].castShadow == 1)
			{
				shadow = CalculateSpotShadow
				(
					fragPos, 
					LightData[i].pos, 
					LightData[i].projectionViewMatrices[0], 
					LightData[i].shadowMapCameraFar, 
					LightData[i].shadowAtlasCoord[0],
					LightData[i].shadowAtlasResRatio, 
					LightData[i].shadowAtlasLayer[0], 
					LightData[i].PCFSamples, 
					LightData[i].PCFRadius, 
					LightData[i].BleedingReduction,
					LightData[i].shadowBias
				);
			}

			irradiance += Lo * shadow * intensity * radiusCheck * attenuation;
		}
	}

	return irradiance;
}
 
vec3 PBRLightingDeferred(vec3 fragPos, vec3 normal, vec3 fragToEye, vec3 viewCamPos, vec3 albedo, float metallic, float roughness)
{
	float shadow = 1.0;
	vec3 irradiance = vec3(0.0);
	float lightDataIndex = 0.0;

	// Directional lights with shadows
	for (lightDataIndex = shadowDirLightsInterval.x; lightDataIndex < shadowDirLightsInterval.y; lightDataIndex += dirShadowLightDataSize)
	{
		vec3 lightDir = -DirLightDirection(s_texture13, lightDataIndex, lightDataTextureWidth);
		vec3 color = DirLightColor(s_texture13, lightDataIndex, lightDataTextureWidth);
		float intensity = DirLightIntensity(s_texture13, lightDataIndex, lightDataTextureWidth);

		// lighting
		vec3 Lo = PBR(fragPos, normal, fragToEye, albedo, metallic, roughness, lightDir, color * intensity);

		mat4 pv = DirLightProjViewMatrix(s_texture13, lightDataIndex, lightDataTextureWidth);
		vec2 shadowAtlasCoord = DirLightShadowAtlasCoord(s_texture13, lightDataIndex, lightDataTextureWidth);
		float shadowAtlasResRatio = DirLightShadowAtlasResRatio(s_texture13, lightDataIndex, lightDataTextureWidth);
		float shadowAtlasLayer = DirLightShadowAtlasLayer(s_texture13, lightDataIndex, lightDataTextureWidth);
		int softShadows = DirLightSoftShadows(s_texture13, lightDataIndex, lightDataTextureWidth);
		int PCFSamples = DirLightPCFSamples(s_texture13, lightDataIndex, lightDataTextureWidth);
		float PCFRadius = DirLightPCFRadius(s_texture13, lightDataIndex, lightDataTextureWidth);
		float lbr = DirLightBleedReduction(s_texture13, lightDataIndex, lightDataTextureWidth);
		float shadowBias = DirLightShadowBias(s_texture13, lightDataIndex, lightDataTextureWidth);

		// shadow
		float shadow = CalculateDirectionalShadow
		(
			fragPos, 
			viewCamPos, 
			pv, 
			shadowAtlasCoord, 
			shadowAtlasResRatio,
			shadowAtlasLayer, 
			PCFSamples, 
			PCFRadius, 
			lbr, 
			shadowBias
		);

		irradiance += Lo * shadow;
	}

	// Directional lights with no shadows
	for (lightDataIndex = nonShadowDirLightsInterval.x; lightDataIndex < nonShadowDirLightsInterval.y; lightDataIndex += dirNonShadowLightDataSize)
	{
		vec3 lightDir = -DirLightDirection(s_texture13, lightDataIndex, lightDataTextureWidth);
		vec3 color = DirLightColor(s_texture13, lightDataIndex, lightDataTextureWidth);
		float intensity = DirLightIntensity(s_texture13, lightDataIndex, lightDataTextureWidth);

		// lighting
		vec3 Lo = PBR(fragPos, normal, fragToEye, albedo, metallic, roughness, lightDir, color * intensity);

		irradiance += Lo;
	}

	// Point lights with shadows
	for (lightDataIndex = shadowPointLightsInterval.x; lightDataIndex < shadowPointLightsInterval.y; lightDataIndex += pointShadowLightDataSize)
	{
		vec3 lightPos = PointLightPosition(s_texture13, lightDataIndex, lightDataTextureWidth);
		float radius = PointLightRadius(s_texture13, lightDataIndex, lightDataTextureWidth);
		vec3 color = PointLightColor(s_texture13, lightDataIndex, lightDataTextureWidth);
		float intensity = PointLightIntensity(s_texture13, lightDataIndex, lightDataTextureWidth);

		float shadowCameraFar = PointLightShadowCameraFar(s_texture13, lightDataIndex, lightDataTextureWidth);
		vec2 shadowAtlasCoord = PointLightShadowAtlasCoord(s_texture13, lightDataIndex, lightDataTextureWidth);
		float shadowAtlasResRatio = PointLightShadowAtlasResRatio(s_texture13, lightDataIndex, lightDataTextureWidth);
		float shadowAtlasLayer = PointLightShadowAtlasLayer(s_texture13, lightDataIndex, lightDataTextureWidth);
		int softShadows = PointLightSoftShadows(s_texture13, lightDataIndex, lightDataTextureWidth);
		int PCFSamples = PointLightPCFSamples(s_texture13, lightDataIndex, lightDataTextureWidth);
		float PCFRadius = PointLightPCFRadius(s_texture13, lightDataIndex, lightDataTextureWidth);
		float lightBleedReduction = PointLightBleedReduction(s_texture13, lightDataIndex, lightDataTextureWidth);
		float shadowBias = PointLightShadowBias(s_texture13, lightDataIndex, lightDataTextureWidth);

		// radius check and attenuation
		float lightDistance = length(lightPos - fragPos);
		float radiusCheck = RadiusCheck(radius, lightDistance);
		float attenuation = Attenuation(lightDistance, radius, 1.0, 0.09, 0.032);

		// lighting
		vec3 lightDir = normalize(lightPos - fragPos);
		vec3 Lo = PBR(fragPos, normal, fragToEye, albedo, metallic, roughness, lightDir, color * intensity);

		// shadow
		float shadow = 1.0;

		irradiance += Lo * shadow * attenuation * radiusCheck;
	}

	// Point lights with no shadows
	for (lightDataIndex = nonShadowPointLightsInterval.x; lightDataIndex < nonShadowPointLightsInterval.y; lightDataIndex += pointNonShadowLightDataSize)
	{
		vec3 lightPos = PointLightPosition(s_texture13, lightDataIndex, lightDataTextureWidth);
		float radius = PointLightRadius(s_texture13, lightDataIndex, lightDataTextureWidth);
		vec3 color = PointLightColor(s_texture13, lightDataIndex, lightDataTextureWidth);
		float intensity = PointLightIntensity(s_texture13, lightDataIndex, lightDataTextureWidth);

		// radius check and attenuation
		float lightDistance = length(lightPos - fragPos);
		float radiusCheck = RadiusCheck(radius, lightDistance);
		float attenuation = Attenuation(lightDistance, radius, 1.0, 0.09, 0.032);

		// lighting
		vec3 lightDir = normalize(lightPos - fragPos);
		vec3 Lo = PBR(fragPos, normal, fragToEye, albedo, metallic, roughness, lightDir, color * intensity);

		irradiance += Lo * attenuation * radiusCheck;
	}

	// Spot lights with shadows
	for (lightDataIndex = shadowSpotLightsInterval.x; lightDataIndex < shadowSpotLightsInterval.y; lightDataIndex += spotShadowLightDataSize)
	{
		vec3 color = SpotLightColor(s_texture13, lightDataIndex, lightDataTextureWidth);
		vec3 lightPos = SpotLightPosition(s_texture13, lightDataIndex, lightDataTextureWidth);
		vec3 dir = SpotLightDirection(s_texture13, lightDataIndex, lightDataTextureWidth);
		float radius = SpotLightRadius(s_texture13, lightDataIndex, lightDataTextureWidth);
		float innAngle = SpotLightInnerAngle(s_texture13, lightDataIndex, lightDataTextureWidth);
		float outAngle = SpotLightOuterAngle(s_texture13, lightDataIndex, lightDataTextureWidth);
		float lightIntensity = SpotLightIntensity(s_texture13, lightDataIndex, lightDataTextureWidth);

		mat4 projView = SpotLightProjViewMatrix(s_texture13, lightDataIndex, lightDataTextureWidth);
		float far = SpotLightShadowCameraFar(s_texture13, lightDataIndex, lightDataTextureWidth);
		vec2 shadowAtlasCoord = SpotLightShadowAtlasCoord(s_texture13, lightDataIndex, lightDataTextureWidth);
		float shadowAtlasResRatio = SpotLightShadowAtlasResRatio(s_texture13, lightDataIndex, lightDataTextureWidth);
		float shadowAtlasLayer = SpotLightShadowAtlasLayer(s_texture13, lightDataIndex, lightDataTextureWidth);
		int softShadows = SpotLightSoftShadows(s_texture13, lightDataIndex, lightDataTextureWidth);
		int PCFSamples = SpotLightPCFSamples(s_texture13, lightDataIndex, lightDataTextureWidth);
		float PCFRadius = SpotLightPCFRadius(s_texture13, lightDataIndex, lightDataTextureWidth);
		float lbr = SpotLightBleedReduction(s_texture13, lightDataIndex, lightDataTextureWidth);
		float shadowBias = SpotLightShadowBias(s_texture13, lightDataIndex, lightDataTextureWidth);

		// radius check and attenuation
		vec3 fragToLight = lightPos - fragPos;
		float lightDistance = length(fragToLight);
		float radiusCheck = RadiusCheck(radius, lightDistance);
		float attenuation = Attenuation(lightDistance, radius, 1.0, 0.09, 0.032);

		// Lighting angle and falloff
		float theta = dot(-normalize(fragToLight), dir);
		float epsilon = innAngle - outAngle;
		float intensity = clamp((theta - outAngle) / epsilon, 0.0, 1.0);

		// lighting
		vec3 lightDir = normalize(-dir);
		vec3 Lo = PBR(fragPos, normal, fragToEye, albedo, metallic, roughness, lightDir, color * lightIntensity);

		// shadow
		float shadow = CalculateSpotShadow
		(
			fragPos, 
			lightPos, 
			projView, 
			far, 
			shadowAtlasCoord,
			shadowAtlasResRatio, 
			shadowAtlasLayer, 
			PCFSamples, 
			PCFRadius, 
			lbr, 
			shadowBias
		);

		irradiance += Lo * shadow * intensity * radiusCheck * attenuation;
	}

	// Spot lights with no shadows
	for (lightDataIndex = nonShadowSpotLightsInterval.x; lightDataIndex < nonShadowSpotLightsInterval.y; lightDataIndex += spotNonShadowLightDataSize)
	{
		vec3 color = SpotLightColor(s_texture13, lightDataIndex, lightDataTextureWidth);
		vec3 lightPos = SpotLightPosition(s_texture13, lightDataIndex, lightDataTextureWidth);
		vec3 dir = SpotLightDirection(s_texture13, lightDataIndex, lightDataTextureWidth);
		float radius = SpotLightRadius(s_texture13, lightDataIndex, lightDataTextureWidth);
		float innAngle = SpotLightInnerAngle(s_texture13, lightDataIndex, lightDataTextureWidth);
		float outAngle = SpotLightOuterAngle(s_texture13, lightDataIndex, lightDataTextureWidth);
		float lightIntensity = SpotLightIntensity(s_texture13, lightDataIndex, lightDataTextureWidth);

		// radius check and attenuation
		vec3 fragToLight = lightPos - fragPos;
		float lightDistance = length(fragToLight);
		float radiusCheck = RadiusCheck(radius, lightDistance);
		float attenuation = Attenuation(lightDistance, radius, 1.0, 0.09, 0.032);

		// Lighting angle and falloff
		float theta = dot(-normalize(fragToLight), dir);
		float epsilon = innAngle - outAngle;
		float intensity = clamp((theta - outAngle) / epsilon, 0.0, 1.0);

		// lighting
		vec3 lightDir = normalize(-dir);
		vec3 Lo = PBR(fragPos, normal, fragToEye, albedo, metallic, roughness, lightDir, color * lightIntensity);

		irradiance += Lo * intensity * radiusCheck * attenuation;
	}

	return irradiance;
}

// Vectors should be normalized (except for color)
vec3 PhongDiffuse(vec3 normal, vec3 fragToLight, vec3 color)
{
	float diff = max(dot(normal, fragToLight), 0.0);
	vec3 diffuse = diff * color;
	return diffuse;
}

// Vectors should be normalized (except for color)
vec3 BlinnPhongSpecular(vec3 fragToLight, vec3 fragToEye, vec3 normal, float shininess, float strength, vec3 color)
{
	vec3 halfwayDir = normalize(fragToLight + fragToEye);  
	float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
	vec3 specular = strength * spec * color;
	return specular;
}

void PointLightBlinnPhong(vec3 fragToLight, vec3 fragToEye, vec3 normal, vec3 color, float radius, out vec3 diffuse, out vec3 specular)
{
	vec3 l = normalize(fragToLight);

	// No need calculation for the fragments outside of the light radius
	float radiusCheck = RadiusCheck(radius, length(fragToLight));

	float attenuation = Attenuation(length(fragToLight), radius, 1.0, 0.09, 0.032);
	diffuse = PhongDiffuse(normal, l, color);
	specular = BlinnPhongSpecular(l, fragToEye, normal, 32.0, 0.4, color);

	diffuse  *= attenuation * radiusCheck;
	specular *= attenuation * radiusCheck;
}

void DirectionalLightBlinnPhong(vec3 fragToLight, vec3 fragToEye, vec3 normal, vec3 color, out vec3 diffuse, out vec3 specular)
{
	diffuse = PhongDiffuse(normal, fragToLight, color);
	specular = BlinnPhongSpecular(fragToLight, fragToEye, normal, 32.0, 0.4, color);
}

void SpotLightBlinnPhong
(
	vec3 fragToLight, 
	vec3 fragToEye, 
	vec3 normal, 
	vec3 color, 
	vec3 direction, 
	float radius,
	float innerAngle, 
	float outerAngle, 
	out vec3 diffuse, 
	out vec3 specular
)
{
	vec3 fragToLightNorm = normalize(fragToLight);
	float fragToLightDist = length(fragToLight);

	// No need calculation for the fragments outside of the light radius
	float radiusCheck = RadiusCheck(radius, fragToLightDist);

	float attenuation = Attenuation(fragToLightDist, radius, 1.0, 0.09, 0.032);
	diffuse = PhongDiffuse(normal, fragToLightNorm, color);
	specular = BlinnPhongSpecular(fragToLightNorm, fragToEye, normal, 32.0, 0.4, color);

	// Lighting angle and falloff
	float theta = dot(-fragToLightNorm, direction);
	float epsilon = innerAngle - outerAngle;
	float intensity = clamp((theta - outerAngle) / epsilon, 0.0, 1.0);

	diffuse *= intensity * radiusCheck * attenuation;
	specular *= intensity * radiusCheck * attenuation;
}

vec3 BlinnPhongLighting(vec3 fragPos, float viewPosDepth, vec3 normal, vec3 fragToEye, vec3 viewCamPos)
{
	float shadow = 1.0;
	vec3 irradiance = vec3(0.0);

	for (int ii = 0; ii < activeCount; ii++)
	{
		int div = ii / 4;
		int rem = ii % 4;
		int i = activeLightIndices[div][rem];

		shadow = 1.0;
		vec3 diffuse = vec3(0.0);
		vec3 specular = vec3(0.0);
		if (LightData[i].type == 2) // Point light
		{
			// Light
			PointLightBlinnPhong(LightData[i].pos - fragPos, fragToEye, normal, LightData[i].color, LightData[i].radius, diffuse, specular);

			// Shadow
			if (LightData[i].castShadow == 1)
			{
				shadow = CalculatePointShadow
				(
					fragPos, 
					LightData[i].pos, 
					LightData[i].shadowMapCameraFar, 
					LightData[i].shadowAtlasCoord, 
					LightData[i].shadowAtlasResRatio,
					LightData[i].shadowAtlasLayer, 
					LightData[i].PCFSamples, 
					LightData[i].PCFRadius, 
					LightData[i].BleedingReduction, 
					LightData[i].shadowBias
				);
			}
		}
		else if (LightData[i].type == 1) // Directional light
		{
			// Light
			DirectionalLightBlinnPhong(-LightData[i].dir, fragToEye, normal, LightData[i].color, diffuse, specular);

			// Shadow
			float depth = abs(viewPosDepth);
			if (LightData[i].castShadow == 1)
			{
				int cascadeOfThisPixel = 0;
				if (LightData[i].numOfCascades > 3 && depth > cascadeDistances[3])
				{
					cascadeOfThisPixel = 3;
				}
				else if (LightData[i].numOfCascades > 2 && depth > cascadeDistances[2])
				{
					cascadeOfThisPixel = 2;
				}
				else if (LightData[i].numOfCascades > 1 && depth > cascadeDistances[1])
				{
					cascadeOfThisPixel = 1;
				}

				shadow = CalculateDirectionalShadow
				(
					fragPos, 
					viewCamPos, 
					LightData[i].projectionViewMatrices[cascadeOfThisPixel], 
					LightData[i].shadowAtlasCoord[cascadeOfThisPixel].xy / shadowAtlasSize,
					LightData[i].shadowAtlasResRatio,	
					LightData[i].shadowAtlasLayer[cascadeOfThisPixel].x, 
					LightData[i].PCFSamples, 
					LightData[i].PCFRadius,
					LightData[i].BleedingReduction, 
					LightData[i].shadowBias
				);
			}		
		}
		else if (LightData[i].type == 3) // Spot light
		{
			// Light
			SpotLightBlinnPhong
			(
				LightData[i].pos - fragPos, 
				fragToEye, 
				normal, 
				LightData[i].color, 
				LightData[i].dir, 
				LightData[i].radius,
				LightData[i].innAngle, 
				LightData[i].outAngle, 
				diffuse, 
				specular
			);

			// Shadow
			if (LightData[i].castShadow == 1)
			{
				shadow = CalculateSpotShadow(
					fragPos, 
					LightData[i].pos, 
					LightData[i].projectionViewMatrices[0], 
					LightData[i].shadowMapCameraFar, 
					LightData[i].shadowAtlasCoord[0],
					LightData[i].shadowAtlasResRatio, 
					LightData[i].shadowAtlasLayer[0], 
					LightData[i].PCFSamples, 
					LightData[i].PCFRadius, 
					LightData[i].BleedingReduction,
					LightData[i].shadowBias
				);
			}
		}

		irradiance += (diffuse + specular) * LightData[i].intensity * shadow;
	}

	return irradiance;
}

#endif

	-->
	</source>
</shader>