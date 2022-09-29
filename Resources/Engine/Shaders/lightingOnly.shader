<shader>
	<type name = "fragmentShader" />
	<uniform name = "LightData" />
	<uniform name = "CamData" />
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
			float normalBias[12];
			float shadowFixedBias[12];
			float shadowSlopedBias[12];
			float shadowMapCamFarPlane[12];
			float PCFSampleHalfSize[12];
			float PCFSampleDistance[12];
			float PCFUnitSampleDistance[12];
			int PCFKernelSize[12];
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
		uniform sampler2D s_texture0;
		uniform samplerCube s_texture7;

		uniform float UseIbl;
		uniform float IblIntensity;

		in vec3 v_pos;
		in vec3 v_normal;
		in vec2 v_texture;

		out vec4 fragColor;

		float CalculateDirectionalShadow(vec3 pos, int index, int dirIndex, vec3 normal)
		{
			vec3 lightDir = normalize(LightData.pos[index] - pos);

			float normalBias = 
			LightData.normalBias[index] * (1.0 - abs(dot(lightDir, normal)));

			vec4 fragPosForLight = LightData.projectionViewMatrix[index]
			* vec4(pos + normal * LightData.normalBias[index], 1.0);

			vec3 projCoord = fragPosForLight.xyz;

			// Transform to [0, 1] range
			projCoord = projCoord * 0.5 + 0.5;

			if (projCoord.z < 0.0 || projCoord.z > 1.0)
			{
				return 1.0;
			}

			// Get depth of the current fragment according to lights view
			float currentDepth = projCoord.z;

			float shadow = 0.0;
			vec2 texelSize = 1.0 / vec2(textureSize(LightData.dirAndSpotLightShadowMap[dirIndex], 0));
			float size = LightData.PCFSampleHalfSize[index];
			float speed = LightData.PCFSampleDistance[index];
			float unit = LightData.PCFUnitSampleDistance[index];

			// PCF
			for (float i = -size; i <= size; i+=speed)
			{
				for (float j = -size; j <= size; j+=speed)
				{
					float shadowSample = texture(LightData.dirAndSpotLightShadowMap[dirIndex],
					projCoord.xy + vec2(i, j) * texelSize).r;
					shadow += currentDepth - size * max(texelSize.x, texelSize.y) - LightData.shadowFixedBias[index]
					> shadowSample ? 0.0 : unit;
				}
			}

			return shadow;
		}

		float CalculateSpotShadow(vec3 pos, int index, int spotIndex, vec3 normal)
		{
			vec3 lightToFrag = pos - LightData.pos[index];
			vec3 lightDir = normalize(-lightToFrag);

			float normalBias = 
			LightData.normalBias[index] * (1.0 - abs(dot(lightDir, normal)));

			vec4 fragPosForLight = LightData.projectionViewMatrix[index]
			* vec4(pos + normal * normalBias, 1.0);

			// Projection divide
			vec3 projCoord = fragPosForLight.xyz / fragPosForLight.w;

			float currentDepth = length(lightToFrag);

			float shadow = 0.0;

			// Transform to [0, 1] range
			projCoord = projCoord * 0.5 + 0.5;

			float bias = max(LightData.shadowFixedBias[index],
			LightData.shadowSlopedBias[index] * (1.0 - dot(normal, lightDir)));

			vec2 texelSize = 1.0 / vec2(textureSize(LightData.dirAndSpotLightShadowMap[spotIndex], 0));
			float size = LightData.PCFSampleHalfSize[index];
			float speed = LightData.PCFSampleDistance[index];
			float unit = LightData.PCFUnitSampleDistance[index];

			// PCF
			for (float i = -size; i <= size; i+=speed)
			{
				for (float j = -size; j <= size; j+=speed)
				{
					float shadowSample = texture(LightData.dirAndSpotLightShadowMap[spotIndex],
					projCoord.xy + vec2(i, j) * texelSize).r;
					// Convert to [0 1] range
					shadowSample *= LightData.shadowMapCamFarPlane[index];
					shadow += currentDepth - size * max(texelSize.x, texelSize.y) - bias
					> shadowSample ? 0.0 : unit;
				}
			}

			return shadow;
		}

		float CalculatePointShadow(vec3 pos, int index, int pointIndex, vec3 normal)
		{
			vec3 lightToFrag = pos - LightData.pos[index];
			vec3 lightDir = normalize(-lightToFrag);

			float normalBias = 
			LightData.normalBias[index] * (1.0 - abs(dot(lightDir, normal)));

			float currentDepth = length(pos + normal * LightData.normalBias[index] - LightData.pos[index]);

			float bias = max(LightData.shadowFixedBias[index],
			LightData.shadowSlopedBias[index] * (1.0 - dot(normal, lightDir)));

			float shadow = 0.0;
			float radius = 2.0 * LightData.PCFSampleHalfSize[index];
			int size = LightData.PCFKernelSize[index];

			// PCF
			for (int i = 0; i < size; i++)
			{
				float shadowSample = texture(LightData.pointLightShadowMap[pointIndex],
				lightToFrag + cubemapSampleDirections[i] * radius).r;
				// Recover from [0 1] range
				shadowSample *= LightData.shadowMapCamFarPlane[index];
				shadow += currentDepth - radius - bias > shadowSample ? 0.0 : 1.0;
			}
			shadow = shadow / float(size);

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

			fragColor = vec4(irradiance, 1.0);
		}
	-->
	</source>
</shader>