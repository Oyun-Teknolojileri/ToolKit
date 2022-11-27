<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision highp float;
		
		uniform sampler2D s_texture0;
		in vec2 v_texture;
		
		uniform float Gamma;
		out vec4 fragColor;

		void main()
		{
			vec2 uv = vec2(v_texture.x, 1.0 - v_texture.y);
			fragColor = texture(s_texture0, uv);
			fragColor.rgb = pow(fragColor.rgb, vec3(1.0/Gamma));
		}
	-->
	</source>
</shader>