<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		out vec4 fragColor;
		in vec3 v_pos;

		uniform vec3 topColor;
		uniform vec3 middleColor;
		uniform vec3 bottomColor;
		uniform float exponent;

		void main()
		{
			vec3 texCoord = normalize(v_pos);
			float d = texCoord.y;
			float s = sign(d);
			vec3 gradColor = s < 0.0 ? bottomColor : topColor;
			float gradAmount = pow(abs(d), exponent);
			vec3 color = mix(middleColor, gradColor, gradAmount);
			fragColor = vec4(color, 1.0);
		}
	-->
	</source>
</shader>