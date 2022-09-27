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
		uniform uint maxLightIterationCount;

		in vec3 v_pos;

		out vec4 fragColor;

		vec3 GetHeatMap(float weight)
		{
			// [Source]: https://www.shadertoy.com/view/llKGWG
			
			// Modulate the red component.
			float r = smoothstep(0.5f, 0.8f, weight);
			if (weight >= 0.90f)
			{
				r *= (1.1f - weight) * 5.0f;
			}
			
			// Modulate the green component.
			float g = 0.0f;
			if (weight > 0.7f)
			{
				g = smoothstep(1.0f, 0.7f, weight);
			}
			else
			{
				g = smoothstep(0.0f, 0.7f, weight);
			}
			
			// Modulate the blue component.
			float b = smoothstep(1.0f, 0.0f, weight);
			if (weight <= 0.3f)
			{
				b *= weight / 0.3f;
			}

			return vec3(r, g, b);
		};
		vec3 GetHeatMap(float value, float max_value){
			return GetHeatMap(min(float(value) / float(max_value), 1.0f));
		};


		void main()
		{
			int dirAndSpotLightShadowCount = 0;
			int pointLightShadowCount = 0;
			int sampledShadowCount = 0;

			vec3 color = GetHeatMap(float(LightData.activeCount), float(8));
			fragColor = vec4(color, 1.0);
		}
	-->
	</source>
</shader>