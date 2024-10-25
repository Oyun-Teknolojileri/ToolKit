<shader>
	<type name = "fragmentShader" />
	<define name = "TextureArray" val = "0,1" />
	<define name = "KernelSize" val = "7,3,9" />
	<source>
	<!--
		#version 300 es
		precision highp float;
		precision highp sampler2DArray;

		uniform vec3 BlurScale;
		uniform sampler2D s_texture0;
		uniform sampler2DArray s_texture1;

		in vec3 v_texture;
		out vec4 fragColor;
		
		void main()
		{
			vec4 color = vec4(0.0);

		#if TextureArray == 1

				color += texture(s_texture1, v_texture + vec3(	vec2(-3.0) * BlurScale.xy, v_texture.z)	) * (1.0 / 64.0);
				color += texture(s_texture1, v_texture + vec3(	vec2(-2.0) * BlurScale.xy, v_texture.z)	) * (6.0 / 64.0);
				color += texture(s_texture1, v_texture + vec3(	vec2(-1.0) * BlurScale.xy, v_texture.z)	) * (15.0 / 64.0);
				color += texture(s_texture1, v_texture + vec3(	vec2(0.0) * BlurScale.xy, v_texture.z)	) * (20.0 / 64.0);
				color += texture(s_texture1, v_texture + vec3(	vec2(1.0) * BlurScale.xy, v_texture.z)	) * (15.0 / 64.0);
				color += texture(s_texture1, v_texture + vec3(	vec2(2.0) * BlurScale.xy, v_texture.z)	) * (6.0 / 64.0);
				color += texture(s_texture1, v_texture + vec3(	vec2(3.0) * BlurScale.xy, v_texture.z)	) * (1.0 / 64.0);

		#else

				color += texture(s_texture0, v_texture.xy + vec2(-3.0) * BlurScale.xy) * (1.0 / 64.0);
				color += texture(s_texture0, v_texture.xy + vec2(-2.0) * BlurScale.xy) * (6.0 / 64.0);
				color += texture(s_texture0, v_texture.xy + vec2(-1.0) * BlurScale.xy) * (15.0 / 64.0);
				color += texture(s_texture0, v_texture.xy + vec2(0.0) * BlurScale.xy) * (20.0 / 64.0);
				color += texture(s_texture0, v_texture.xy + vec2(1.0) * BlurScale.xy) * (15.0 / 64.0);
				color += texture(s_texture0, v_texture.xy + vec2(2.0) * BlurScale.xy) * (6.0 / 64.0);
				color += texture(s_texture0, v_texture.xy + vec2(3.0) * BlurScale.xy) * (1.0 / 64.0);

		#endif

		  fragColor = color;
		}
	-->
	</source>
</shader>