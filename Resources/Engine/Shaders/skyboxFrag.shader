<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision lowp float;
		precision lowp sampler2D;

		out vec4 fragColor;
		in vec3 v_pos;

		uniform samplerCube s_texture6;

		void main()
		{
			vec3 color = texture(s_texture6, v_pos).rgb;

			fragColor = vec4(color, 1.0);
		}
	-->
	</source>
</shader>