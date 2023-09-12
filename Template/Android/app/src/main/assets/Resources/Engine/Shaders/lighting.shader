<shader>
	<type name = "includeShader" />
	<include name = "textureUtil.shader" />
	<include name = "shadow.shader" />
	<include name = "lightDataTextureUtils.shader" />
	<include name = "pbr.shader" />
	<uniform name = "LightData" />

	<source>
	<!--
// Fixed Declaretions
struct _LightData
{
  /*
  Type
  1 : Directional light
  2 : Point light
  3 : Spot light
  */
	int type[16];
	vec3 pos[16];
	vec3 dir[16];
	vec3 color[16];
	float intensity[16];
	float radius[16];
	float outAngle[16];
	float innAngle[16];
	int activeCount;

	mat4 projectionViewMatrix[16];
	float shadowMapCameraFar[16];
	int castShadow[16];
	int PCFSamples[16];
	float PCFRadius[16];
	float BleedingReduction[16];
	int softShadows[16];
	float shadowAtlasLayer[16];
	float shadowAtlasResRatio[16];
	vec2 shadowAtlasCoord[16]; // Between 0 and 1
	float shadowBias[16];
};
uniform _LightData LightData;

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

// Returns uv coordinates and layers such as: vec3(u,v,layer)
// https://kosmonautblog.wordpress.com/2017/03/25/shadow-filtering-for-pointlights/

// Can be improved:
// https://stackoverflow.com/questions/53115467/how-to-implement-texturecube-using-6-sampler2d
vec3 UVWToUVLayer(vec3 vec)
{
	/*
		layer:
		  0       1       2       3       4       5
		pos X   neg X   pos Y   neg Y   pos Z   neg Z
	*/
	float layer;
	vec2 coord;

	if (abs(vec.x) >= abs(vec.y) && abs(vec.x) >= abs(vec.z))
	{
		if (vec.x > 0.0)
		{
			layer = 0.0;
			vec /= vec.x;
			coord = -vec.zy;
		}
		else
		{
			layer = 1.0;
			vec.y = -vec.y;
			vec /= vec.x;
			coord = -vec.zy;
		}
	}
	else if (abs(vec.y) >= abs(vec.x) && abs(vec.y) >= abs(vec.z))
	{
		if (vec.y > 0.0)
		{
			layer = 2.0;
			vec /= vec.y;
			coord = vec.xz;
		}
		else
		{
			layer = 3.0;
			vec.x = -vec.x;
			vec /= vec.y;
			coord = vec.xz;
		}
	}
	else
	{
		if (vec.z > 0.0)
		{
			layer = 4.0;
			vec.y = -vec.y;
			vec /= -vec.z;
			coord = -vec.xy;
		}
		else
		{
			layer = 5.0;
			vec /= -vec.z;
			coord = -vec.xy;
		}
	}

	coord = (coord + vec2(1.0)) * 0.5;
	return vec3(coord, layer);
}

bool EpsilonEqual(float a, float b, float eps)
{
	return abs(a - b) < eps;
}

float CalculateDirectionalShadow(vec3 pos, mat4 lightProjView, vec2 shadowAtlasCoord,
	float shadowAtlasResRatio, float shadowAtlasLayer, int PCFSamples, float PCFRadius, float lightBleedReduction, float shadowBias)
{
	vec4 fragPosForLight = lightProjView * vec4(pos, 1.0);
	vec3 projCoord = fragPosForLight.xyz;
	projCoord = projCoord * 0.5 + 0.5;
	if (projCoord.z < 0.0 || projCoord.z > 1.0)
	{
		return 1.0;
	}

	// Get depth of the current fragment according to lights view
	float currFragDepth = projCoord.z;

	vec2 startCoord = shadowAtlasCoord;
	float resRatio = shadowAtlasResRatio;
	vec3 coord = vec3(startCoord + resRatio * projCoord.xy, shadowAtlasLayer);

	if (PCFSamples >= 1)
	{
		return PCFFilterShadow2D(s_texture8, coord, startCoord, startCoord + resRatio,
		PCFSamples, PCFRadius * shadowAtlasResRatio,
		projCoord.z, lightBleedReduction, shadowBias);
	}
	else
	{
		coord.xy = ClampTextureCoordinates(coord.xy, startCoord, startCoord + resRatio);
		vec2 moments = texture(s_texture8, coord).xy;
		return ChebyshevUpperBound(moments, projCoord.z, lightBleedReduction, shadowBias);
	}

	return 1.0;
}

float CalculateSpotShadow(vec3 pos, vec3 lightPos, mat4 lightProjView, float shadowCameraFar, vec2 shadowAtlasCoord,
 float shadowAtlasResRatio, float shadowAtlasLayer, int PCFSamples, float PCFRadius, float lightBleedReduction, float shadowBias)
{
	vec4 fragPosForLight = lightProjView * vec4(pos, 1.0);
	vec3 projCoord = fragPosForLight.xyz / fragPosForLight.w;
	projCoord = projCoord * 0.5 + 0.5;

	vec3 lightToFrag = pos - lightPos;
	float currFragDepth = length(lightToFrag) / shadowCameraFar;

	vec2 startCoord = shadowAtlasCoord;
	vec3 coord = vec3(startCoord + shadowAtlasResRatio * projCoord.xy, shadowAtlasLayer);

	if (PCFSamples >= 1)
	{
		return PCFFilterShadow2D(s_texture8, coord, startCoord, startCoord + shadowAtlasResRatio,
		PCFSamples, PCFRadius * shadowAtlasResRatio, currFragDepth, lightBleedReduction, shadowBias);
	}
	else
	{
		coord.xy = ClampTextureCoordinates(coord.xy, startCoord, startCoord + shadowAtlasResRatio);
		vec2 moments = texture(s_texture8, coord).xy;
		return ChebyshevUpperBound(moments, currFragDepth, lightBleedReduction, shadowBias);
	}

	return 1.0;
}

float CalculatePointShadow(vec3 pos, vec3 lightPos, float shadowCameraFar, vec2 shadowAtlasCoord, float shadowAtlasResRatio,
	float shadowAtlasLayer, int PCFSamples, float PCFRadius, float lightBleedReduction, float shadowBias)
{
	vec3 lightToFrag = pos - lightPos;
	float currFragDepth = length(lightToFrag) / shadowCameraFar;

	vec2 startCoord = shadowAtlasCoord;
	float resRatio = shadowAtlasResRatio;

	vec3 coord = UVWToUVLayer(lightToFrag);
	coord.xy = startCoord + resRatio * coord.xy;
	coord.z = shadowAtlasLayer + coord.z;

	if (PCFSamples >= 1)
	{
		return PCFFilterShadow2D(s_texture8, coord, startCoord, startCoord + resRatio,
		PCFSamples, PCFRadius * resRatio, currFragDepth, lightBleedReduction, shadowBias);
	}
	else
	{
		coord.xy = ClampTextureCoordinates(coord.xy, startCoord, startCoord + resRatio);
		vec2 moments = texture(s_texture8, coord).xy;
		return ChebyshevUpperBound(moments, currFragDepth, lightBleedReduction, shadowBias);
	}
	
	return 1.0;
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

vec3 PBRLighting(vec3 fragPos, vec3 normal, vec3 fragToEye, vec3 albedo, float metallic, float roughness)
{
	vec3 irradiance = vec3(0.0);

	for (int i = 0; i < LightData.activeCount; i++)
	{
		if (LightData.type[i] == 2) // Point light
		{
			// radius check and attenuation
			float lightDistance = length(LightData.pos[i] - fragPos);
			float radiusCheck = RadiusCheck(LightData.radius[i], lightDistance);
			float attenuation = Attenuation(lightDistance, LightData.radius[i], 1.0, 0.09, 0.032);

			// lighting
			vec3 lightDir = normalize(LightData.pos[i] - fragPos);
			vec3 Lo = PBR(fragPos, normal, fragToEye, albedo, metallic, roughness, lightDir, LightData.color[i] * LightData.intensity[i]);

			// shadow
			float shadow = 1.0;
			if (LightData.castShadow[i] == 1)
			{
				shadow = CalculatePointShadow(fragPos, LightData.pos[i], LightData.shadowMapCameraFar[i], LightData.shadowAtlasCoord[i], LightData.shadowAtlasResRatio[i],
						LightData.shadowAtlasLayer[i], LightData.PCFSamples[i], LightData.PCFRadius[i], LightData.BleedingReduction[i], LightData.shadowBias[i]);
			}

			irradiance += Lo * shadow * attenuation * radiusCheck;
		}
		else if (LightData.type[i] == 1) // Directional light
		{
			// lighting
			vec3 lightDir = normalize(-LightData.dir[i]);
			vec3 Lo = PBR(fragPos, normal, fragToEye, albedo, metallic, roughness, lightDir, LightData.color[i] * LightData.intensity[i]);

			// shadow
			float shadow = 1.0;
			if (LightData.castShadow[i] == 1)
			{
				shadow = CalculateDirectionalShadow(fragPos, LightData.projectionViewMatrix[i], LightData.shadowAtlasCoord[i], LightData.shadowAtlasResRatio[i],
					LightData.shadowAtlasLayer[i], LightData.PCFSamples[i], LightData.PCFRadius[i], LightData.BleedingReduction[i],
					LightData.shadowBias[i]);
			}

			irradiance += Lo * shadow;
		}
		else // if (LightData.type[i] == 3) Spot light
		{
			// radius check and attenuation
			vec3 fragToLight = LightData.pos[i] - fragPos;
			float lightDistance = length(fragToLight);
			float radiusCheck = RadiusCheck(LightData.radius[i], lightDistance);
			float attenuation = Attenuation(lightDistance, LightData.radius[i], 1.0, 0.09, 0.032);

			// Lighting angle and falloff
			float theta = dot(-normalize(fragToLight), LightData.dir[i]);
			float epsilon = LightData.innAngle[i] - LightData.outAngle[i];
			float intensity = clamp((theta - LightData.outAngle[i]) / epsilon, 0.0, 1.0);

			// lighting
			vec3 lightDir = normalize(-LightData.dir[i]);
			vec3 Lo = PBR(fragPos, normal, fragToEye, albedo, metallic, roughness, lightDir, LightData.color[i] * LightData.intensity[i]);

			// shadow
			float shadow = 1.0;
			if (LightData.castShadow[i] == 1)
			{
				shadow = CalculateSpotShadow(fragPos, LightData.pos[i], LightData.projectionViewMatrix[i], LightData.shadowMapCameraFar[i], LightData.shadowAtlasCoord[i],
					LightData.shadowAtlasResRatio[i], LightData.shadowAtlasLayer[i], LightData.PCFSamples[i], LightData.PCFRadius[i], LightData.BleedingReduction[i],
					LightData.shadowBias[i]);
			}

			irradiance += Lo * shadow * intensity * radiusCheck * attenuation;
		}
	}

	return irradiance;
}
 
vec3 PBRLightingDeferred(vec3 fragPos, vec3 normal, vec3 fragToEye, vec3 albedo, float metallic, float roughness)
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
		float shadow = CalculateDirectionalShadow(fragPos, pv, shadowAtlasCoord, shadowAtlasResRatio,
		                                          shadowAtlasLayer, PCFSamples, PCFRadius, lbr, shadowBias);

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
		float shadow = CalculatePointShadow(fragPos, lightPos, shadowCameraFar, shadowAtlasCoord, shadowAtlasResRatio,
			shadowAtlasLayer, PCFSamples, PCFRadius, lightBleedReduction, shadowBias);

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
		float shadow = CalculateSpotShadow(fragPos, lightPos, projView, far, shadowAtlasCoord,
			shadowAtlasResRatio, shadowAtlasLayer, PCFSamples, PCFRadius, lbr, shadowBias);

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
	-->
	</source>
</shader>