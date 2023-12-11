<shader>
	<type name = "fragmentShader" />
	<include name = "lighting.shader" />
	<uniform name = "UseIbl" />
	<uniform name = "IblIntensity" />
	<uniform name = "IBLIrradianceMap" />
	<source>
	<!--
		#version 300 es
		precision mediump float;

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
			vec3 color = GetHeatMap(float(LightData.activeCount), float(8));
			fragColor = vec4(color, 1.0);
		}
	-->
	</source>
</shader>