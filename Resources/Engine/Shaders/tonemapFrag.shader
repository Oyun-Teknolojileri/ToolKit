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

		vec3 uncharted2_tonemap_partial(vec3 x)
		{
				float A = 0.15f;
				float B = 0.50f;
				float C = 0.10f;
				float D = 0.20f;
				float E = 0.02f;
				float F = 0.30f;
				return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
		}

		vec3 uncharted2_filmic(vec3 v)
		{
				float exposure_bias = 2.0f;
				vec3 curr = uncharted2_tonemap_partial(v * exposure_bias);

				vec3 W = vec3(11.2f);
				vec3 white_scale = vec3(1.0f) / uncharted2_tonemap_partial(W);
				return curr * white_scale;
		}

		void main()
		{
			vec2 uv = vec2(v_texture.x, 1.0 - v_texture.y);
			fragColor = texture(s_texture0, uv);
			if(UseAcesTonemapper > 0u){
				fragColor.rgb = ACESFilm(fragColor.rgb);
			}
			else{
				fragColor.rgb = uncharted2_filmic(fragColor.rgb);
			}
		}
	-->
	</source>
</shader>