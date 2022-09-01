<shader>
	<type name = "fragmentShader" />
	<uniform name = "LightData" />
	<uniform name = "CamData" />
	<uniform name = "Color" />
	<uniform name = "UseIbl" />
	<uniform name = "IblIntensity" />
	<uniform name = "IBLIrradianceMap" />
	<source>
	<!--
		#version 300 es
		precision mediump float;

		// Fixed Declaretions
		struct _LightData
		{
			/*
			Type
				1 : Directional light
				2 : Point light
				3 : Spot light
			*/
			int type[12];
			vec3 pos[12];
			vec3 dir[12];
			vec3 color[12];
			float intensity[12];
			float radius[12];
			float outAngle[12];
			float innAngle[12];
			int activeCount;

			mat4 projectionViewMatrix[12];
			sampler2D dirAndSpotLightShadowMap[4];
			samplerCube pointLightShadowMap[4];
			int castShadow[12];
			float shadowBias[12];
			float shadowMapCamFarPlane[12];
			int shadowPCFKernelHalfSize[12];
		};
		const int maxPointLightShadows = 4;
		const int maxDirAndSpotLightShadows = 4;

		const vec3 cubemapSampleDirections[20] = vec3[]
		(
			vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
			vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
			vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
			vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
			vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
		);

		struct _CamData
		{
			vec3 pos;
			vec3 dir;
			float farPlane;
		};

		uniform _LightData LightData;
		uniform _CamData CamData;
		uniform vec4 Color;

		uniform samplerCube s_texture7;
		uniform float UseIbl;
		uniform float IblIntensity;

		in vec3 v_pos;
		in vec3 v_normal;
		in vec2 v_texture;

		out vec4 fragColor;

		// https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
		float CalculateDirectionalShadow(vec3 pos, int index, int dirIndex, vec3 normal)
		{
			vec4 fragPosForLight = LightData.projectionViewMatrix[index] * vec4(pos, 1.0);

			// Projection divide
			vec3 projCoord = fragPosForLight.xyz / fragPosForLight.w;

			// Transform to [0, 1] range
			projCoord = projCoord * 0.5 + 0.5;

			// Get depth of the current fragment according to lights view
			float currentDepth = projCoord.z;

			float shadow = 0.0;
			vec2 texelSize = 1.0 / vec2(textureSize(LightData.dirAndSpotLightShadowMap[dirIndex], 0));
			int size = LightData.shadowPCFKernelHalfSize[index];
			// PCF
			for (int i = -size; i < size; i++)
			{
				for (int j = -size; j <= size; j++)
				{
					float shadowSample = texture(LightData.dirAndSpotLightShadowMap[dirIndex],
					projCoord.xy + vec2(i, j) * texelSize).r;
					shadow += currentDepth - LightData.shadowBias[index] > shadowSample ? 0.0 : 1.0;
				}
			}
			shadow = shadow / float((size * 2 + 1) * (size * 2 + 1));

			return shadow;
		}

		float CalculateSpotShadow(vec3 pos, int index, int spotIndex, vec3 normal)
		{
			vec4 fragPosForLight = LightData.projectionViewMatrix[index] * vec4(pos, 1.0);

			// Projection divide
			vec3 projCoord = fragPosForLight.xyz / fragPosForLight.w;

			vec3 lightToFrag = pos - LightData.pos[index];
			float currentDepth = length(lightToFrag);
			// Convert to [0 1] range
			currentDepth = currentDepth / LightData.shadowMapCamFarPlane[index];

			float shadow = 0.0;

			// Transform to [0, 1] range
			projCoord = projCoord * 0.5 + 0.5;

			vec2 texelSize = 1.0 / vec2(textureSize(LightData.dirAndSpotLightShadowMap[spotIndex], 0));
			int size = LightData.shadowPCFKernelHalfSize[index];
			// PCF
			for (int i = -size; i < size; i++)
			{
				for (int j = -size; j <= size; j++)
				{
					float shadowSample = texture(LightData.dirAndSpotLightShadowMap[spotIndex],
					projCoord.xy + vec2(i, j) * texelSize).r;
					shadow += currentDepth - LightData.shadowBias[index] > shadowSample ? 0.0 : 1.0;
				}
			}
			shadow = shadow / float((size * 2 + 1) * (size * 2 + 1));

			return shadow;
		}

		float CalculatePointShadow(vec3 pos, int index, int pointIndex, vec3 normal)
		{
			vec3 lightToFrag = pos - LightData.pos[index];

			float currentDepth = length(lightToFrag);

			// Shadow bias
			vec3 lightDir = normalize(-lightToFrag);
			float bias = LightData.shadowBias[index];
			bias = max(bias * 10.0 * (1.0 - dot(normal, lightDir)), 0.1);

			float shadow = 0.0;
			float radius = 0.03;
			// PCF
			for (int i = 0; i < 20; i++)
			{
				float shadowSample = texture(LightData.pointLightShadowMap[pointIndex],
				lightToFrag + cubemapSampleDirections[i] * radius).r;
				// Recover from [0 1] range
				shadowSample *= LightData.shadowMapCamFarPlane[index];
				shadow += currentDepth - bias > shadowSample ? 0.0 : 1.0;
			}
			shadow = shadow / 20.0;

			return shadow;
		}

		void main()
		{
			int dirAndSpotLightShadowCount = 0;
			int pointLightShadowCount = 0;

			vec3 n = normalize(v_normal);
			vec3 e = normalize(CamData.pos - v_pos);

			float shadow = 1.0;
			vec3 irradiance = vec3(0.0);
			for (int i = 0; i < LightData.activeCount; i++)
			{
				int maxShadowCheck = 1;
				shadow = 1.0;
				vec3 ambient = vec3(0.0);
				vec3 diffuse = vec3(0.0);
				vec3 specular = vec3(0.0);
				if (LightData.type[i] == 2) // Point light
				{
					vec3 fragToLight = LightData.pos[i] - v_pos;

					// No need calculation for the fragments outside of the light radius
					float radiusCheck = clamp(LightData.radius[i] - length(fragToLight), 0.0, 1.0);
					radiusCheck = ceil(radiusCheck);

					vec3 l = normalize(fragToLight);

					// Attenuation
					float constant = 1.0;
					float linear = 0.09;
					float quadratic = 0.032;
					float distance = length(fragToLight);
					float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));

					// Decrase attenuation heavily near radius
					float cutOff = 0.5;
					attenuation *= clamp
					(
						cutOff * distance + LightData.radius[i] * 0.5 - distance,
						0.0,
						1.0
					);

					// Diffuse
					float diff = max(dot(n, l), 0.0);
					diffuse = diff * LightData.color[i];

					// Specular
					float specularStrength = 0.4;
					vec3 halfwayDir = normalize(l + e);  
					float spec = pow(max(dot(n, halfwayDir), 0.0), 32.0);
					specular = specularStrength * spec * LightData.color[i];

					diffuse  *= attenuation * radiusCheck;
					specular *= attenuation * radiusCheck;

					bool maxShadowCheck = maxPointLightShadows > pointLightShadowCount;
					if (maxShadowCheck && LightData.castShadow[i] == 1)
					{
						shadow = CalculatePointShadow(v_pos, i, pointLightShadowCount, n);
						pointLightShadowCount += 1;
					}
				}
				else if (LightData.type[i] == 1) // Directional light
				{
					vec3 l = -LightData.dir[i];

					// Diffuse 
					float diff = max(dot(n, l), 0.0);
					diffuse = diff * LightData.color[i];

					// Specular
					float specularStrength = 0.4;
					vec3 halfwayDir = normalize(l + e);  
					float spec = pow(max(dot(n, halfwayDir), 0.0), 32.0);
					specular = specularStrength * spec * LightData.color[i];

					// Ambient
					float ambientStrength = 0.01;
					ambient = ambientStrength * LightData.color[i];

					bool maxShadowCheck = maxDirAndSpotLightShadows > dirAndSpotLightShadowCount;
					if (maxShadowCheck && LightData.castShadow[i] == 1)
					{
						shadow = CalculateDirectionalShadow(v_pos, i, dirAndSpotLightShadowCount, n);
						dirAndSpotLightShadowCount += 1;
					}
				}
				else if (LightData.type[i] == 3) // Spot light
				{
					vec3 fragToLight = LightData.pos[i] - v_pos;

					// No need calculation for the fragments outside of the light radius
					float radiusCheck = clamp(LightData.radius[i] - length(fragToLight), 0.0, 1.0);
					radiusCheck = ceil(radiusCheck);

					vec3 l = -LightData.dir[i];

					// Attenuation
					float constant = 1.0;
					float linear = 0.09;
					float quadratic = 0.032;
					float distance = length(fragToLight);
					float attenuation = 1.0 /
					(constant + linear * distance + quadratic * (distance * distance));

					// Decrase attenuation heavily near radius
					float cutOff = 0.5;
					attenuation *= clamp
					(
						cutOff * distance + LightData.radius[i] * 0.5 - distance,
						0.0,
						1.0
					);

					// Diffuse
					float diff = max(dot(n, l), 0.0);
					diffuse = diff * LightData.color[i];

					// Specular
					float specularStrength = 0.4;
					vec3 halfwayDir = normalize(l + e);  
					float spec = pow(max(dot(n, halfwayDir), 0.0), 32.0);
					specular = specularStrength * spec * LightData.color[i];

					vec3 lightToFrag = normalize(v_pos -  LightData.pos[i]);
					float theta = dot(lightToFrag, -l);
					float epsilon =  LightData.innAngle[i] - LightData.outAngle[i];
					float intensity = clamp((theta - LightData.outAngle[i]) / epsilon, 0.0, 1.0);

					diffuse *= intensity * radiusCheck * attenuation;
					specular *= intensity * radiusCheck * attenuation;

					bool maxShadowCheck = maxDirAndSpotLightShadows > dirAndSpotLightShadowCount;
					if (maxShadowCheck && LightData.castShadow[i] == 1)
					{
						shadow = CalculateSpotShadow(v_pos, i, dirAndSpotLightShadowCount, n);
						dirAndSpotLightShadowCount += 1;
					}
				}

				irradiance += (ambient + diffuse + specular) * LightData.intensity[i] * shadow;
			}

			vec3 iblIrradiance = UseIbl * texture(s_texture7, n).rgb;
			irradiance += iblIrradiance * IblIntensity;

			fragColor = vec4(irradiance * Color.xyz, Color.a);
		}
	-->
	</source>
</shader>