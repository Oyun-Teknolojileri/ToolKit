<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		in vec2 v_texture;

		uniform vec3 BlurScale;
		uniform sampler2D s_texture0;

		out vec4 fragColor;

		void main()
		{
			vec4 color = vec4(0.0);
			for (int i = -2; i < 2; ++i)
			{
				color += texture(s_texture0, v_texture + vec2(i) * BlurScale.xy);
			}
			color /= 4.0;
			fragColor = color;
		}
	-->
	</source>
</shader>