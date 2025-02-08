<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		uniform samplerCube s_texture6;
		uniform int lodLevel;

		in vec2 v_texture;
		out vec4 fragColor;

		const float PI = 3.14159265359;

		void main()
		{
			float phi = v_texture.x * 2.0 * PI;
			float theta = v_texture.y * PI;

			vec3 dir;
			dir.x = sin(theta) * cos(phi);
			dir.y = cos(theta);
			dir.z = sin(theta) * sin(phi);

			vec3 color = textureLod(s_texture6, normalize(dir), float(lodLevel)).rgb;
			fragColor = vec4(color, 1.0);
		}
	-->
	</source>
</shader>
