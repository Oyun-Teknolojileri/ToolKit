<shader>
	<type name = "fragmentShader" />
	<include name = "lighting.shader" />
	<include name = "ibl.shader" />
	<include name = "AO.shader" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		// Position buffer
		uniform sampler2D s_texture9;
		// Normal buffer
		uniform sampler2D s_texture10;
		// Color buffer
		uniform sampler2D s_texture11;

		uniform int aoEnabled;

		uniform vec3 camPos;

		in vec2 v_texture;

		out vec4 fragColor;

		void main()
		{
			vec2 texCoord = vec2(v_texture.x, 1.0 - v_texture.y);
			vec3 position = texture(s_texture9, texCoord).rgb;
			vec3 normal = texture(s_texture10, texCoord).rgb;
			vec3 color = texture(s_texture11, texCoord).rgb;

			vec3 n = normalize(normal);
			vec3 e = normalize(camPos - position);

			vec3 irradiance = BlinnPhongLightingDeferred(position, n, e);

			irradiance += IblIrradiance(n);

			float ambientOcclusion;
			if (aoEnabled == 1)
			{
				ambientOcclusion = AmbientOcclusion();
			}
			else
			{
				ambientOcclusion = 1.0;
			}

			fragColor = vec4(irradiance * color * ambientOcclusion, 1.0);
		}
	-->
	</source>
</shader>