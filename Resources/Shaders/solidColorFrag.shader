<shader>
	<type name = "fragmentShader" />
	<uniform name = "Color" />
	<source>
		#version 300 es
		precision mediump float;
		
		out vec4 v_fragColor;
		uniform vec3 Color;
		
		void main()
		{
			v_fragColor = vec4(Color, 1.0);
		}
	</source>
</shader>