<shader>
	<type name = "fragmentShader" />
	<uniform name = "LightData" />
	<uniform name = "CamData" />
	<uniform name = "Color" />
	<source>
	<!--
		#version 300 es
		precision mediump float;

		// Fixed Declaretions
		struct _LightData
		{
			vec3 pos[8];
			vec3 dir[8];
			vec3 color[8];
			float intensity[8];
			int activeCount;
		};

		struct _CamData
		{
			vec3 pos;
			vec3 dir;
		};

		uniform _LightData LightData;
		uniform _CamData CamData;
		uniform vec3 Color;

		in vec3 v_pos;
		in vec3 v_normal;
		in vec2 v_texture;

		out vec4 fragColor;

		void main()
		{
			vec3 n = normalize(v_normal);
			vec3 e = normalize(CamData.pos - v_pos);

			vec3 irradiance = vec3(0.0);
			for (int i = 0; i < LightData.activeCount; i++)
			{
				vec3 l = -LightData.dir[i];

				// ambient
				float ambientStrength = 0.1;
				vec3 ambient = ambientStrength * LightData.color[i];

				// diffuse 
				float diff = max(dot(n, l), 0.0);
				vec3 diffuse = diff * LightData.color[i];

				// specular
				float specularStrength = 0.5;
				vec3 reflectDir = reflect(-l, n);
				float spec = pow(max(dot(e, reflectDir), 0.0), 32.0);
				vec3 specular = specularStrength * spec * LightData.color[i];

				irradiance += (ambient + diffuse + specular) * LightData.intensity[i];
			}

			fragColor = vec4(irradiance, 1.0) * vec4(Color, 1.0f);
		}
	-->
	</source>
</shader>