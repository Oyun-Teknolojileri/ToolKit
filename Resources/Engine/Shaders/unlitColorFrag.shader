<shader>
	<type name = "fragmentShader" />
	<uniform name = "Color" />
	<source>
	<!--
		#version 300 es
		precision mediump float;
		
		uniform vec4 Color;
		out vec4 fragColor;

		void main()
		{
		  fragColor = Color;
		}
	-->
	</source>
</shader>