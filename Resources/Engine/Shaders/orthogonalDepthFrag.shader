<shader>
	<type name = "fragmentShader" />
	<include name = "VSM.shader" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "ColorAlpha" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		in vec2 v_texture;

		uniform int DiffuseTextureInUse;
		uniform sampler2D s_texture0;
		uniform float ColorAlpha;

		out vec4 fragColor;

		void main()
		{
			float alpha = 1.0;
			if (DiffuseTextureInUse == 1)
			{
				alpha = texture(s_texture0, v_texture).a;
			}
			else
			{
				alpha = ColorAlpha;
			}

#ifdef ENABLE_DISCARD_PIXEL
			if (alpha < 0.1)
			{
				discard;
			}
#endif

			fragColor = vec4(ComputeMoments(gl_FragCoord.z), 0.0, 0.0);
		}
	-->
	</source>
</shader>