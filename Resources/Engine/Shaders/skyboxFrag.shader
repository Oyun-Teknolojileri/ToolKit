<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		out vec4 fragColor;
		in vec3 v_pos;

		uniform samplerCube cubeMap;

		void main()
		{
		    vec3 color = texture(cubeMap, v_pos).rgb;

		    fragColor = vec4(color, 1.0);
		}
	-->
	</source>
</shader>