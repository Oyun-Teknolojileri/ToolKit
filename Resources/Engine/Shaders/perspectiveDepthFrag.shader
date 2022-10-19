<shader>
	<type name = "fragmentShader" />
	<include name = "VSM.shader" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "ColorAlpha" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		in vec4 v_pos;
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

	    vec2 lightDistance = ComputeMoments(length(v_pos.xyz));
	    fragColor = vec4(lightDistance, 0.0, 0.0);
		}
	-->
	</source>
</shader>