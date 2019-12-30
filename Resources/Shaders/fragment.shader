<shader>
	<type name = "fragmentShader" />
	<source>
		#version 300 es
		precision mediump float;
		
		in vec3 v_normal;
		in vec2 v_texCoord;
		in vec3 v_bitan;
		out vec4 v_fragColor;
		uniform sampler2D s_texture;
		
		void main()
		{
			vec2 texFixedc = vec2(v_texCoord.x, 1.0 - v_texCoord.y);
			v_fragColor = vec4(v_bitan, 1.0);
		}
	</source>
</shader>