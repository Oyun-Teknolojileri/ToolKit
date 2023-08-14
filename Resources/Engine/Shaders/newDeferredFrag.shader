<shader>
	<type name = "fragmentShader" />
	<include name = "lighting.shader" />
	<include name = "AO.shader" />
	<uniform name = "lightingType" />
	<source>
	<!--
		#version 300 es
		precision highp float;
		precision highp sampler2D;
		precision highp samplerCube;
		precision highp sampler2DArray;

		// Position buffer
		uniform sampler2D s_texture9;
		// Normal buffer
		uniform sampler2D s_texture10;
		// Color buffer
		uniform sampler2D s_texture11;
		// Emissive buffer
		// uniform sampler2D s_texture12;
		// Metallic roughness buffer
		uniform sampler2D s_texture14;
		// ibl buffer
		// uniform sampler2D s_texture16;

		uniform vec3 camPos;

		in vec2 v_texture;

		out vec3 fragColor;

		void main()
		{
			if (v_texture.x > 0.5)
				discard;
			vec2 texCoord = vec2(v_texture.x, 1.0 - v_texture.y);
			vec3 position = texture(s_texture9, texCoord).rgb;
			vec3 normal   = texture(s_texture10, texCoord).rgb;
			vec3 color    = texture(s_texture11, texCoord).rgb;
			vec2 metallicRoughness = texture(s_texture14, texCoord).rg;

			vec3 n = normalize(normal);
			vec3 e = normalize(camPos - position);

			vec3 irradiance = LightingDeferred2(position, n, e, color, metallicRoughness.r, metallicRoughness.g);
			fragColor = irradiance;
		}
	-->
	</source>
</shader>