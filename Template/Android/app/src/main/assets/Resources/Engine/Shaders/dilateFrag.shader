<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision mediump float;
		
		uniform vec4 Color;
		uniform sampler2D u_texture;
		
		in vec2 v_texture;
		out vec4 o_fragColor;
		
		vec2 g_textureSize;
	
		#define R 2
		
		void main()
		{
			vec2 uv = vec2(v_texture.x, 1.0 - v_texture.y);
			float c = texture(u_texture, uv).r;
			if (c == 0.0)
			{
				// Reject inner part of the stencil.
				discard;
			}
			
			g_textureSize = vec2(textureSize(u_texture, 0));
			
			for (int i = -R; i <= R; i++)
			{
				for (int j = -R; j <= R; j++)
				{
					c = texture(u_texture, uv + (vec2(i, j) / g_textureSize)).r;
					if (c == 0.0)
					{
						// If anything in the kernel is 0, this pixel will be colored.
						o_fragColor = vec4(Color.xyz, 1.0);
						return;
					}
				}
			}
			
			discard;
		}
	-->
	</source>
</shader>