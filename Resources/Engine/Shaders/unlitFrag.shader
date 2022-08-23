<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision mediump float;

		uniform sampler2D s_texture0;
		
		in vec2 v_texture;
		out vec4 fragColor;

		void main()
		{
		  fragColor = texture(s_texture0, v_texture);
		}
	-->
	</source>
</shader>