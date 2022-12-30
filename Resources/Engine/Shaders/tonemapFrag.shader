<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision highp float;
		
		uniform sampler2D s_texture0;
		in vec2 v_texture;
		
		uniform uint UseAcesTonemapper;
		out vec4 fragColor;

		vec3 ACESFilm(vec3 x)
		{
			float a = 2.51f;
			float b = 0.03f;
			float c = 2.43f;
			float d = 0.59f;
			float e = 0.14f;
			return clamp((x*(a*x+b))/(x*(c*x+d)+e),0.0,1.0);
		}

		void main()
		{
			vec2 uv = vec2(v_texture.x, 1.0 - v_texture.y);
			fragColor = texture(s_texture0, uv);

			if(UseAcesTonemapper > 0u){
				fragColor.rgb = ACESFilm(fragColor.rgb);
			}
			else{
				fragColor.xyz = fragColor.xyz / (fragColor.xyz + vec3(1.0));
			}
		}
	-->
	</source>
</shader>