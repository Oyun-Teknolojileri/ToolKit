<shader>
	<type name = "fragmentShader" />
	<uniform name = "Color" />
	<source>
	<!--
		#version 300 es
		precision mediump float;
		
		uniform vec3 Color;
		out vec4 fragColor;

		void main()
		{
		  fragColor = vec4(Color, 1.0f);
		}
	-->
	</source>
</shader>