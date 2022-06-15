<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision mediump float;

		uniform sampler2D s_texture;
		
		in vec2 v_texture;
		out vec4 fragColor;

		void main()
		{
		  fragColor = texture(s_texture, v_texture);
		}
	-->
	</source>
</shader>