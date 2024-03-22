<shader>
	<type name = "fragmentShader" />
	<include name = "gammaFunctions.shader" />
	<source>
	<!--
		#version 300 es
		precision highp float;
		
		uniform sampler2D s_texture0;
		in vec2 v_texture;

		out vec4 fragColor;

		void main()
		{
			vec2 uv = vec2(v_texture.x, 1.0 - v_texture.y);
			fragColor = texture(s_texture0, uv);
			fragColor.xyz = Gamma(fragColor.xyz);
		}
	-->
	</source>
</shader>