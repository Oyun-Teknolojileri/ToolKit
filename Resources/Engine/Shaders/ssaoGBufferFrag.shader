<shader>
	<type name = "fragmentShader" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "ColorAlpha" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		layout (location = 0) out vec4 fragColor;
		layout (location = 1) out vec4 fragNormal;

		in vec3 v_pos;
		in vec3 v_normal;
		in vec2 v_texture;

		uniform int DiffuseTextureInUse;
		uniform sampler2D s_texture0;
		uniform float colorAlpha;

		void main()
		{
			float alpha = float(1 - DiffuseTextureInUse) * colorAlpha
				+ float(DiffuseTextureInUse) * texture(s_texture0, v_texture).a;
			if (alpha < 0.1)
			{
				discard;
			}

		  fragColor = vec4(v_pos, 1.0);
		  fragNormal = vec4(normalize(v_normal), 1.0);
		}
	-->
	</source>
</shader>