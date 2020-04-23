<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision mediump float;
		
		in vec3 v_normal;
		in vec2 v_texCoord;
		in vec3 v_bitan;
		out vec4 v_fragColor;
		uniform sampler2D s_texture;
		
		void main()
		{
			vec3 n = v_normal * 0.8;  
			for (int i = 0; i < 3; i++)
			{
				if (n[i] < 0.0)
				{
					n[i] *= -0.3;
				}
			}
			v_fragColor = vec4(n, 1.0);
		}
	-->
	</source>
</shader>