<shader>
	<type name = "includeShader" />
	<include name = "textureUtil.shader" />
	<include name = "shadow.shader" />
	<include name = "lightDataTextureUtils.shader" />
	<uniform name = "LightData" />
	<uniform name = "metallic" />
	<uniform name = "roughness" />
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
			float lightBleedingReduction[16];
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
		///

		uniform float metallic;
		uniform float roughness;

		const int maxLights = 16;
		const float PI = 3.14159265359;

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
			float shadowAtlasResRatio, float shadowAtlasLayer, int softShadows, int PCFSamples, float PCFRadius, float lightBleedReduction, float shadowBias)
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

			if (softShadows == 1)
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
		 float shadowAtlasResRatio, float shadowAtlasLayer, int softShadows, int PCFSamples, float PCFRadius, float lightBleedReduction, float shadowBias)
		{
			vec4 fragPosForLight = lightProjView * vec4(pos, 1.0);
			vec3 projCoord = fragPosForLight.xyz / fragPosForLight.w;
			projCoord = projCoord * 0.5 + 0.5;

			vec3 lightToFrag = pos - lightPos;
			float currFragDepth = length(lightToFrag) / shadowCameraFar;

			vec2 startCoord = shadowAtlasCoord;
			vec3 coord = vec3(startCoord + shadowAtlasResRatio * projCoord.xy, shadowAtlasLayer);

			if (softShadows == 1)
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
			float shadowAtlasLayer, int softShadows, int PCFSamples, float PCFRadius, float lightBleedReduction, float shadowBias)
		{
			vec3 lightToFrag = pos - lightPos;
			float currFragDepth = length(lightToFrag) / shadowCameraFar;

			vec2 startCoord = shadowAtlasCoord;
			float resRatio = shadowAtlasResRatio;

			vec3 coord = UVWToUVLayer(lightToFrag);
			coord.xy = startCoord + resRatio * coord.xy;
			coord.z = shadowAtlasLayer + coord.z;

			if (softShadows == 1)
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

		void SpotLightBlinnPhong(vec3 fragToLight, vec3 fragToEye, vec3 normal, vec3 color, vec3 direction, float radius,
			float innerAngle, float outerAngle, out vec3 diffuse, out vec3 specular)
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

		vec3 BlinnPhongLighting(vec3 fragPos, vec3 normal, vec3 fragToEye)
		{
			float shadow = 1.0;
			vec3 irradiance = vec3(0.0);

			int lightCount = 0;
			for (int i = 0; i < LightData.activeCount; i++)
			{
				shadow = 1.0;
				vec3 diffuse = vec3(0.0);
				vec3 specular = vec3(0.0);
				if (LightData.type[i] == 2) // Point light
				{
					// Light
					PointLightBlinnPhong(LightData.pos[i] - fragPos, fragToEye, normal, LightData.color[i], LightData.radius[i], diffuse, specular);

					// Shadow
					bool maxShadowCheck = maxLights > lightCount;
					if (maxShadowCheck && LightData.castShadow[i] == 1)
					{
						shadow = CalculatePointShadow(fragPos, LightData.pos[i], LightData.shadowMapCameraFar[i], LightData.shadowAtlasCoord[i], LightData.shadowAtlasResRatio[i],
							LightData.shadowAtlasLayer[i], LightData.softShadows[i], LightData.PCFSamples[i], LightData.PCFRadius[i], LightData.lightBleedingReduction[i], LightData.shadowBias[i]);
						lightCount += 1;
					}
				}
				else if (LightData.type[i] == 1) // Directional light
				{
					// Light
					DirectionalLightBlinnPhong(-LightData.dir[i], fragToEye, normal, LightData.color[i], diffuse, specular);

					// Shadow
					bool maxShadowCheck = maxLights > lightCount;
					if (maxShadowCheck && LightData.castShadow[i] == 1)
					{
						shadow = CalculateDirectionalShadow(fragPos, LightData.projectionViewMatrix[i], LightData.shadowAtlasCoord[i], LightData.shadowAtlasResRatio[i],
							LightData.shadowAtlasLayer[i], LightData.softShadows[i], LightData.PCFSamples[i], LightData.PCFRadius[i], LightData.lightBleedingReduction[i],
							LightData.shadowBias[i]);
						lightCount += 1;
					}		
				}
				else if (LightData.type[i] == 3) // Spot light
				{
					// Light
					SpotLightBlinnPhong(LightData.pos[i] - fragPos, fragToEye, normal, LightData.color[i], LightData.dir[i], LightData.radius[i],
						LightData.innAngle[i], LightData.outAngle[i], diffuse, specular);

					// Shadow
					bool maxShadowCheck = maxLights > lightCount;
					if (maxShadowCheck && LightData.castShadow[i] == 1)
					{
						shadow = CalculateSpotShadow(fragPos, LightData.pos[i], LightData.projectionViewMatrix[i], LightData.shadowMapCameraFar[i], LightData.shadowAtlasCoord[i],
							LightData.shadowAtlasResRatio[i], LightData.shadowAtlasLayer[i], LightData.softShadows[i], LightData.PCFSamples[i], LightData.PCFRadius[i], LightData.lightBleedingReduction[i],
							LightData.shadowBias[i]);
						lightCount += 1;
					}
				}

				irradiance += (diffuse + specular) * LightData.intensity[i] * shadow;
			}

			return irradiance;
		}

		vec3 BlinnPhongLightingDeferred(vec3 fragPos, vec3 normal, vec3 fragToEye)
		{
			float shadow = 1.0;
			vec3 irradiance = vec3(0.0);
			float lightDataIndex = 0.0;

			// Directional lights with shadows
			for (lightDataIndex = shadowDirLightsInterval.x; lightDataIndex < shadowDirLightsInterval.y; lightDataIndex += dirShadowLightDataSize)
			{
				vec3 diffuse = vec3(0.0);
				vec3 specular = vec3(0.0);

				vec3 dir = DirLightDirection(s_texture13, lightDataIndex, lightDataTextureWidth);
				vec3 col = DirLightColor(s_texture13, lightDataIndex, lightDataTextureWidth);
				float intensity = DirLightIntensity(s_texture13, lightDataIndex, lightDataTextureWidth);
				DirectionalLightBlinnPhong(-dir, fragToEye, normal, col, diffuse, specular);

				mat4 pv = DirLightProjViewMatrix(s_texture13, lightDataIndex, lightDataTextureWidth);
				vec2 shadowAtlasCoord = DirLightShadowAtlasCoord(s_texture13, lightDataIndex, lightDataTextureWidth);
				float shadowAtlasResRatio = DirLightShadowAtlasResRatio(s_texture13, lightDataIndex, lightDataTextureWidth);
				float shadowAtlasLayer = DirLightShadowAtlasLayer(s_texture13, lightDataIndex, lightDataTextureWidth);
				int softShadows = DirLightSoftShadows(s_texture13, lightDataIndex, lightDataTextureWidth);
				int PCFSamples = DirLightPCFSamples(s_texture13, lightDataIndex, lightDataTextureWidth);
				float PCFRadius = DirLightPCFRadius(s_texture13, lightDataIndex, lightDataTextureWidth);
				float lbr = DirLightBleedReduction(s_texture13, lightDataIndex, lightDataTextureWidth);
				float shadowBias = DirLightShadowBias(s_texture13, lightDataIndex, lightDataTextureWidth);
				shadow = CalculateDirectionalShadow(fragPos, pv, shadowAtlasCoord, shadowAtlasResRatio, shadowAtlasLayer, softShadows, PCFSamples, PCFRadius, lbr, shadowBias);

				irradiance += (diffuse + specular) * intensity * shadow;
			}

			// Directional lights with no shadows
			for (lightDataIndex = nonShadowDirLightsInterval.x; lightDataIndex < nonShadowDirLightsInterval.y; lightDataIndex += dirNonShadowLightDataSize)
			{
				vec3 diffuse = vec3(0.0);
				vec3 specular = vec3(0.0);

				vec3 dir = DirLightDirection(s_texture13, lightDataIndex, lightDataTextureWidth);
				vec3 col = DirLightColor(s_texture13, lightDataIndex, lightDataTextureWidth);
				float intensity = DirLightIntensity(s_texture13, lightDataIndex, lightDataTextureWidth);
				DirectionalLightBlinnPhong(-dir, fragToEye, normal, col, diffuse, specular);

				irradiance += (diffuse + specular) * intensity;
			}

			// Point lights with shadows
			for (lightDataIndex = shadowPointLightsInterval.x; lightDataIndex < shadowPointLightsInterval.y; lightDataIndex += pointShadowLightDataSize)
			{
				vec3 diffuse = vec3(0.0);
				vec3 specular = vec3(0.0);

				vec3 col = PointLightColor(s_texture13, lightDataIndex, lightDataTextureWidth);
				vec3 pos = PointLightPosition(s_texture13, lightDataIndex, lightDataTextureWidth);
				float radius = PointLightRadius(s_texture13, lightDataIndex, lightDataTextureWidth);
				float intensity = PointLightIntensity(s_texture13, lightDataIndex, lightDataTextureWidth);
				PointLightBlinnPhong(pos - fragPos, fragToEye, normal, col, radius, diffuse, specular);

				float shadowCameraFar = PointLightShadowCameraFar(s_texture13, lightDataIndex, lightDataTextureWidth);
				vec2 shadowAtlasCoord = PointLightShadowAtlasCoord(s_texture13, lightDataIndex, lightDataTextureWidth);
				float shadowAtlasResRatio = PointLightShadowAtlasResRatio(s_texture13, lightDataIndex, lightDataTextureWidth);
				float shadowAtlasLayer = PointLightShadowAtlasLayer(s_texture13, lightDataIndex, lightDataTextureWidth);
				int softShadows = PointLightSoftShadows(s_texture13, lightDataIndex, lightDataTextureWidth);
				int PCFSamples = PointLightPCFSamples(s_texture13, lightDataIndex, lightDataTextureWidth);
				float PCFRadius = PointLightPCFRadius(s_texture13, lightDataIndex, lightDataTextureWidth);
				float lightBleedReduction = PointLightBleedReduction(s_texture13, lightDataIndex, lightDataTextureWidth);
				float shadowBias = PointLightShadowBias(s_texture13, lightDataIndex, lightDataTextureWidth);
				shadow = CalculatePointShadow(fragPos, pos, shadowCameraFar, shadowAtlasCoord, shadowAtlasResRatio, shadowAtlasLayer, softShadows,
					PCFSamples, PCFRadius, lightBleedReduction, shadowBias);

				irradiance += (diffuse + specular) * intensity * shadow;
			}

			// Point lights with no shadows
			for (lightDataIndex = nonShadowPointLightsInterval.x; lightDataIndex < nonShadowPointLightsInterval.y; lightDataIndex += pointNonShadowLightDataSize)
			{
				vec3 diffuse = vec3(0.0);
				vec3 specular = vec3(0.0);

				vec3 col = PointLightColor(s_texture13, lightDataIndex, lightDataTextureWidth);
				vec3 pos = PointLightPosition(s_texture13, lightDataIndex, lightDataTextureWidth);
				float radius = PointLightRadius(s_texture13, lightDataIndex, lightDataTextureWidth);
				float intensity = PointLightIntensity(s_texture13, lightDataIndex, lightDataTextureWidth);
				PointLightBlinnPhong(pos - fragPos, fragToEye, normal, col, radius, diffuse, specular);

				irradiance += (diffuse + specular) * intensity;
			}

			// Spot lights with shadows
			for (lightDataIndex = shadowSpotLightsInterval.x; lightDataIndex < shadowSpotLightsInterval.y; lightDataIndex += spotShadowLightDataSize)
			{
				vec3 diffuse = vec3(0.0);
				vec3 specular = vec3(0.0);

				vec3 col = SpotLightColor(s_texture13, lightDataIndex, lightDataTextureWidth);
				vec3 pos = SpotLightPosition(s_texture13, lightDataIndex, lightDataTextureWidth);
				vec3 dir = SpotLightDirection(s_texture13, lightDataIndex, lightDataTextureWidth);
				float radius = SpotLightRadius(s_texture13, lightDataIndex, lightDataTextureWidth);
				float innAngle = SpotLightInnerAngle(s_texture13, lightDataIndex, lightDataTextureWidth);
				float outAngle = SpotLightOuterAngle(s_texture13, lightDataIndex, lightDataTextureWidth);
				float intensity = SpotLightIntensity(s_texture13, lightDataIndex, lightDataTextureWidth);
				SpotLightBlinnPhong(pos - fragPos, fragToEye, normal, col, dir, radius,	innAngle, outAngle, diffuse, specular);

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
				shadow = CalculateSpotShadow(fragPos, pos, projView, far, shadowAtlasCoord, shadowAtlasResRatio, shadowAtlasLayer, softShadows, PCFSamples, PCFRadius, lbr, shadowBias);

				irradiance += (diffuse + specular) * intensity * shadow;
			}

			// Spot lights with no shadows
			for (lightDataIndex = nonShadowSpotLightsInterval.x; lightDataIndex < nonShadowSpotLightsInterval.y; lightDataIndex += spotNonShadowLightDataSize)
			{
				vec3 diffuse = vec3(0.0);
				vec3 specular = vec3(0.0);

				vec3 col = SpotLightColor(s_texture13, lightDataIndex, lightDataTextureWidth);
				vec3 pos = SpotLightPosition(s_texture13, lightDataIndex, lightDataTextureWidth);
				vec3 dir = SpotLightDirection(s_texture13, lightDataIndex, lightDataTextureWidth);
				float radius = SpotLightRadius(s_texture13, lightDataIndex, lightDataTextureWidth);
				float innAngle = SpotLightInnerAngle(s_texture13, lightDataIndex, lightDataTextureWidth);
				float outAngle = SpotLightOuterAngle(s_texture13, lightDataIndex, lightDataTextureWidth);
				float intensity = SpotLightIntensity(s_texture13, lightDataIndex, lightDataTextureWidth);
				SpotLightBlinnPhong(pos - fragPos, fragToEye, normal, col, dir, radius,	innAngle, outAngle, diffuse, specular);

				irradiance += (diffuse + specular) * intensity;
			}

			return irradiance;
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

		vec3 FresnelSchlick(float cosTheta, vec3 F0)
		{
				return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
		}

		vec3 PointLightPBR(vec3 fragPos, vec3 normal, vec3 fragToEye, vec3 albedo, float metallic, float roughness, vec3 lightPos, vec3 lightColor)
		{
			// Reflectance normal incidence
			vec3 F0 = vec3(0.04);
			F0 = mix(F0, albedo, metallic);

			vec3 lightDir = normalize(lightPos - fragPos);
			vec3 halfway = normalize(lightDir + fragToEye);
			float hXe = clamp(dot(halfway, fragToEye), 0.0, 1.0);
			float nXe = max(dot(normal, fragToEye), 0.0);
			float nXl = max(dot(normal, lightDir), 0.0);
			vec3 radiance = lightColor;

			// Cook-Torrance BRDF
			float NDF = DistributionGGX(normal, halfway, roughness);
			float G = GeometrySmith(normal, fragToEye, lightDir, roughness);
			vec3 F = FresnelSchlick(max(dot(halfway, fragToEye), 0.0), F0);
			vec3 numerator = NDF * G * F;
			float denominator = 4.0 * nXe * nXl + 0.0001; // 0.0001 to prevent divide by zero
			vec3 specular = numerator / denominator;

			vec3 kS = F; // Specular part
			// Energy conservation
			vec3 kD = vec3(1.0) - kS; // Diffuse part
			// Only non-metals have diffuse part. (Also works fine with metallic values between 0-1)
			kD *= 1.0 - metallic;

			// Note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
			// Scale light by light and normal vectors angle
			vec3 outIrradiance = (kD * albedo / PI + specular) * radiance * nXl;

			return outIrradiance;
		}

		vec3 PBRLighting(vec3 fragPos, vec3 normal, vec3 fragToEye, vec3 albedo)
		{
			// TODO Lighting for other light types
			// TODO Shadows

			vec3 irradiance = vec3(0.0);

			for (int i = 0; i < LightData.activeCount; i++)
			{
				if (LightData.type[i] == 2) // Point light
				{
					// TODO radius check

					irradiance += PointLightPBR(fragPos, normal, fragToEye, albedo, metallic, roughness, LightData.pos[i], LightData.color[i] * LightData.intensity[i]);
				}
				else if (LightData.type[i] == 1) // Directional light
				{

				}
				else // if (LightData.type[i] == 3) Spot light
				{

				}
			}

			return irradiance;
		}

	-->
	</source>
</shader>