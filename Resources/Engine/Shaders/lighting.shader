<shader>
	<type name = "includeShader" />
	<include name = "textureUtil.shader" />
	<include name = "shadow.shader" />
	<include name = "lightDataTextureUtils.shader" />
	<include name = "pbr.shader" />
	<uniform name = "shadowDistance" />
	<uniform name = "activeCount" />
	<source>
	<!--

#ifndef LIGHTING_SHADER
#define LIGHTING_SHADER


#define MAX_LIGHT_COUNT 128

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
	int shadowAtlasLayer;

	// Shadow filter
	int PCFSamples;
	float PCFRadius;
	float shadowAtlasResRatio; // shadow map resolution / shadow atlas resolution.

	vec2 shadowAtlasCoord; // Between 0 and 1
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
	int shadowAtlasLayer, 
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
	vec3 sampleCoord = vec3(uvInAtlas, float(shadowAtlasLayer));

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
		PCFRadius / SHADOW_ATLAS_SIZE, // Convert radius in pixel units to uv.
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
	int shadowAtlasLayer, 
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
		PCFRadius / SHADOW_ATLAS_SIZE, // Convert radius in pixel units to uv.
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
	vec2 shadowAtlasCoord, 
	float shadowAtlasResRatio,
	int shadowAtlasLayer, 
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
		PCFRadius / SHADOW_ATLAS_SIZE, // Convert radius in pixel units to uv.
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

	// Decrease attenuation heavily near radius
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
				int numCascade = LightData[i].numOfCascades;
				int cascadeOfThisPixel = numCascade - 1;

				// Cascade selection by depth range.
				for (int ci = 0; ci < numCascade; ci++)
				{
					if (depth < cascadeDistances[ci])
					{
						cascadeOfThisPixel = ci;
						break;
					}
				}

				int layer = 0;
				vec2 coord = vec2(0.0);
				float shadowMapSize = LightData[i].shadowAtlasResRatio * SHADOW_ATLAS_SIZE;
				ShadowAtlasLut(shadowMapSize, LightData[i].shadowAtlasCoord, cascadeOfThisPixel, layer, coord);

				layer += LightData[i].shadowAtlasLayer;

				shadow = CalculateDirectionalShadow
				(
					fragPos, 
					viewCamPos, 
					LightData[i].projectionViewMatrices[cascadeOfThisPixel], 
					coord / SHADOW_ATLAS_SIZE, // Convert the resolution to uv
					LightData[i].shadowAtlasResRatio,	
					layer, 
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
					LightData[i].shadowAtlasCoord / SHADOW_ATLAS_SIZE, // Convert the resolution to uv
					LightData[i].shadowAtlasResRatio, 
					LightData[i].shadowAtlasLayer, 
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

#endif

	-->
	</source>
</shader>