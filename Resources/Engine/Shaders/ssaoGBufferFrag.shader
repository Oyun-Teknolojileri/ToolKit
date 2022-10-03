<shader>
	<type name = "fragmentShader" />
	<uniform name = "Color" />
	<source>
	<!--
		#version 300 es
		precision highp float;
		
		uniform vec4 Color;
		layout (location = 0) out vec4 fragColor;
		layout (location = 1) out vec4 fragNormal;

		in vec3 v_pos;
		in vec3 v_normal;
		in vec2 v_texture;

		void main()
		{
		  fragColor = vec4(v_pos, 1.0);
		  fragNormal = vec4(normalize(v_normal), 1.0);
		}
	-->
	</source>
</shader>