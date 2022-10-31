<shader>
	<type name = "fragmentShader" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "ColorAlpha" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		in float v_depth;
		in vec2 v_texture;

		uniform int DiffuseTextureInUse;
		uniform sampler2D s_texture0;
		uniform float colorAlpha;

		out vec4 fragColor;

		void main()
		{
			float alpha = float(1 - DiffuseTextureInUse) * colorAlpha
				+ float(DiffuseTextureInUse) * texture(s_texture0, v_texture).a;
			if (alpha < 0.1)
			{
				discard;
			}

			fragColor = vec4(v_depth, v_depth, v_depth, 1.0);
		}
	-->
	</source>
</shader>