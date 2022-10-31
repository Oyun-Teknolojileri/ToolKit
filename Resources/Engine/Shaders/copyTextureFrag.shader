<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		in vec2 v_texture;

		uniform sampler2D s_texture0;

		out vec4 fragColor;
		
		void main()
		{
		  fragColor = texture(s_texture0, v_texture).rgba;
		}
	-->
	</source>
</shader>