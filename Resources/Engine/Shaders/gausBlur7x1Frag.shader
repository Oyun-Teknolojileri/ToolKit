<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		in vec2 v_texture;

		uniform vec3 BlurScale;
		uniform sampler2D s_texture0;

		out vec4 fragColor;
		
		void main()
		{
			vec4 color = vec4(0.0);
		  
			color += texture(s_texture0, v_texture + vec2(-3.0) * BlurScale.xy) * (1.0 / 64.0);
			color += texture(s_texture0, v_texture + vec2(-2.0) * BlurScale.xy) * (6.0 / 64.0);
			color += texture(s_texture0, v_texture + vec2(-1.0) * BlurScale.xy) * (15.0 / 64.0);
			color += texture(s_texture0, v_texture + vec2(0.0) * BlurScale.xy) * (20.0 / 64.0);
			color += texture(s_texture0, v_texture + vec2(1.0) * BlurScale.xy) * (15.0 / 64.0);
			color += texture(s_texture0, v_texture + vec2(2.0) * BlurScale.xy) * (6.0 / 64.0);
			color += texture(s_texture0, v_texture + vec2(3.0) * BlurScale.xy) * (1.0 / 64.0);

		  fragColor = color;
		}
	-->
	</source>
</shader>