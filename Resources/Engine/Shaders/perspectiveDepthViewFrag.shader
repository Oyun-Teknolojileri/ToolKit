<shader>
	<type name = "fragmentShader" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "ColorAlpha" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		in vec3 v_pos;
		in vec2 v_texture;

		uniform int DiffuseTextureInUse;
		uniform sampler2D s_texture0;
		uniform float colorAlpha;
		uniform vec3 lightPos;
		uniform float far;

		out vec4 fragColor;

		void main()
		{
			float alpha = float(1 - DiffuseTextureInUse) * colorAlpha
				+ float(DiffuseTextureInUse) * texture(s_texture0, v_texture).a;
			if (alpha < 0.1)
			{
				discard;
			}

			float depth = length(lightPos - v_pos) / far;
			fragColor = vec4(depth, depth, depth, 1.0);
		}
	-->
	</source>
</shader>