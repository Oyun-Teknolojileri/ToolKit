<shader>
	<type name = "includeShader" />
	<include name = "shadow.shader" />
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
			float shadowMapCameraFar[12];
			//samplerCube pointLightShadowMap[4];
			int castShadow[12];
			int PCFSamples[12];
			float PCFRadius[12];
			float lightBleedingReduction[12];
			int softShadows[12];
			int shadowAtlasFirstLayer[12];
			int shadowAtlasLayers[12];
		};
		uniform _LightData LightData;
		
		sampler2DArray shadowAtlas;

		const int maxPointLightShadows = 4;
		const int maxDirAndSpotLightShadows = 4;

		float CalculateDirectionalShadow(vec3 pos, int index, int dirIndex, vec3 normal)
		{
			vec3 lightDir = normalize(LightData.pos[index] - pos);
			vec4 fragPosForLight = LightData.projectionViewMatrix[index] * vec4(pos, 1.0);
			vec3 projCoord = fragPosForLight.xyz;
			projCoord = projCoord * 0.5 + 0.5;
			if (projCoord.z < 0.0 || projCoord.z > 1.0)
			{
				return 1.0;
			}

			// Get depth of the current fragment according to lights view
			float currFragDepth = projCoord.z;

/*
			if (LightData.softShadows[index] == 1)
			{
				return PCFFilterShadow2D(LightData.dirAndSpotLightShadowMap[dirIndex], projCoord.xy,
				LightData.PCFSamples[index], LightData.PCFRadius[index], projCoord.z,
				LightData.lightBleedingReduction[index]);
			}
			else
*/
			{
				vec2 moments = texture(shadowAtlas, vec3(projCoord.xy, LightData.shadowAtlasFirstLayer[index])).xy;
				return ChebyshevUpperBound(moments, projCoord.z, LightData.lightBleedingReduction[index]);
			}
		}

		float CalculateSpotShadow(vec3 pos, int index, int spotIndex, vec3 normal)
		{
			vec4 fragPosForLight = LightData.projectionViewMatrix[index] * vec4(pos, 1.0);
			vec3 projCoord = fragPosForLight.xyz / fragPosForLight.w;
			projCoord = projCoord * 0.5 + 0.5;

			vec3 lightToFrag = pos - LightData.pos[index];
			float currFragDepth = length(lightToFrag) / LightData.shadowMapCameraFar[index];
/*
			if (LightData.softShadows[index] == 1)
			{
				return PCFFilterShadow2D(LightData.dirAndSpotLightShadowMap[spotIndex], projCoord.xy,
				LightData.PCFSamples[index], LightData.PCFRadius[index], currFragDepth,
				LightData.lightBleedingReduction[index]);
			}
			else
*/
			{
				vec2 moments = texture(shadowAtlas, vec3(projCoord.xy, LightData.shadowAtlasFirstLayer[index])).xy;
				return ChebyshevUpperBound(moments, currFragDepth, LightData.lightBleedingReduction[index]);
			}
		}

		float CalculatePointShadow(vec3 pos, int index, int pointIndex, vec3 normal)
		{
			/*
			vec3 lightToFrag = pos - LightData.pos[index];
			float currFragDepth = length(lightToFrag) / LightData.shadowMapCameraFar[index];

			if (LightData.softShadows[index] == 1)
			{
				return PCFFilterShadow3D(LightData.pointLightShadowMap[pointIndex], lightToFrag,
				LightData.PCFSamples[index], LightData.PCFRadius[index], currFragDepth,
				LightData.lightBleedingReduction[index]);
			}
			else
			{
				vec2 moments = texture(LightData.pointLightShadowMap[pointIndex], lightToFrag).xy;
				return ChebyshevUpperBound(moments, currFragDepth, LightData.lightBleedingReduction[index]);
			}
			*/
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

		void PointLightBlinnPhong(int i, vec3 fragToLight, vec3 fragToEye, vec3 normal, out vec3 diffuse, out vec3 specular)
		{
			vec3 l = normalize(fragToLight);

			// No need calculation for the fragments outside of the light radius
			float radiusCheck = RadiusCheck(LightData.radius[i], length(fragToLight));

			float attenuation = Attenuation(length(fragToLight), LightData.radius[i], 1.0, 0.09, 0.032);
			diffuse = PhongDiffuse(normal, l, LightData.color[i]);
			specular = BlinnPhongSpecular(l, fragToEye, normal, 32.0, 0.4, LightData.color[i]);

			diffuse  *= attenuation * radiusCheck;
			specular *= attenuation * radiusCheck;
		}

		void DirectionalLightBlinnPhong(int i, vec3 fragToLight, vec3 fragToEye, vec3 normal, out vec3 diffuse, out vec3 specular)
		{
			diffuse = PhongDiffuse(normal, fragToLight, LightData.color[i]);
			specular = BlinnPhongSpecular(fragToLight, fragToEye, normal, 32.0, 0.4, LightData.color[i]);
		}

		void SpotLightBlinnPhong(int i, vec3 fragToLight, vec3 fragToEye, vec3 normal, out vec3 diffuse, out vec3 specular)
		{
					vec3 fragToLightNorm = normalize(fragToLight);
					float fragToLightDist = length(fragToLight);

					// No need calculation for the fragments outside of the light radius
					float radiusCheck = RadiusCheck(LightData.radius[i], fragToLightDist);
	
					float attenuation = Attenuation(fragToLightDist, LightData.radius[i], 1.0, 0.09, 0.032);
					diffuse = PhongDiffuse(normal, fragToLightNorm, LightData.color[i]);
					specular = BlinnPhongSpecular(fragToLightNorm, fragToEye, normal, 32.0, 0.4, LightData.color[i]);

					// Lighting angle and falloff
					float theta = dot(-fragToLightNorm, LightData.dir[i]);
					float epsilon =  LightData.innAngle[i] - LightData.outAngle[i];
					float intensity = clamp((theta - LightData.outAngle[i]) / epsilon, 0.0, 1.0);

					diffuse *= intensity * radiusCheck * attenuation;
					specular *= intensity * radiusCheck * attenuation;
		}

		vec3 BlinnPhongLighting(vec3 fragPos, vec3 normal, vec3 fragToEye)
		{
			int dirAndSpotLightShadowCount = 0;
			int pointLightShadowCount = 0;

			float shadow = 1.0;
			vec3 irradiance = vec3(0.0);
			for (int i = 0; i < LightData.activeCount; i++)
			{
				shadow = 1.0;
				vec3 diffuse = vec3(0.0);
				vec3 specular = vec3(0.0);
				if (LightData.type[i] == 2) // Point light
				{
					// Light
					PointLightBlinnPhong(i, LightData.pos[i] - fragPos, fragToEye, normal, diffuse, specular);

					// Shadow
					bool maxShadowCheck = maxPointLightShadows > pointLightShadowCount;
					if (maxShadowCheck && LightData.castShadow[i] == 1)
					{
						shadow = CalculatePointShadow(fragPos, i, pointLightShadowCount, normal);
						pointLightShadowCount += 1;
					}
				}
				else if (LightData.type[i] == 1) // Directional light
				{
					// Light
					DirectionalLightBlinnPhong(i, -LightData.dir[i], fragToEye, normal, diffuse, specular);

					// Shadow
					bool maxShadowCheck = maxDirAndSpotLightShadows > dirAndSpotLightShadowCount;
					if (maxShadowCheck && LightData.castShadow[i] == 1)
					{
						shadow = CalculateDirectionalShadow(fragPos, i, dirAndSpotLightShadowCount, normal);
						dirAndSpotLightShadowCount += 1;
					}		
				}
				else if (LightData.type[i] == 3) // Spot light
				{
					// Light
					SpotLightBlinnPhong(i, LightData.pos[i] - fragPos, fragToEye, normal, diffuse, specular);

					// Shadow
					bool maxShadowCheck = maxDirAndSpotLightShadows > dirAndSpotLightShadowCount;
					if (maxShadowCheck && LightData.castShadow[i] == 1)
					{
						shadow = CalculateSpotShadow(fragPos, i, dirAndSpotLightShadowCount, normal);
						dirAndSpotLightShadowCount += 1;
					}
				}

				irradiance += (diffuse + specular) * LightData.intensity[i] * shadow;
			}
			return irradiance;
		}

	-->
	</source>
</shader>