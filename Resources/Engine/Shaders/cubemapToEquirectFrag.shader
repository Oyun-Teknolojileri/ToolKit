<shader>
	<type name = "fragmentShader" />
	<uniform name = "CubeMap" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		uniform samplerCube CubeMap;

		in vec2 uv;
		out vec4 fragColor;

		const float PI = 3.14159265359;

		void main()
		{
			float phi = uv.x * 2.0 * PI;
			float theta = uv.y * PI;

			vec3 dir;
			dir.x = sin(theta) * cos(phi);
			dir.y = cos(theta);
			dir.z = sin(theta) * sin(phi);

			vec3 color = texture(CubeMap, normalize(dir)).rgb;
			fragColor = vec4(color, 1.0);
		}
	-->
	</source>
</shader>
