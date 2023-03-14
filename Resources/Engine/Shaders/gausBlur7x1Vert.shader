<shader>
	<type name = "vertexShader" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		// Fixed Attributes.
		layout (location = 0) in vec3 vPosition;
		layout (location = 1) in vec3 vNormal;
		layout (location = 2) in vec2 vTexture;

		vec3 v_pos;
		out vec2 v_texture;
		
		void main()
		{
		  v_pos.xy = vPosition.xy * 2.0;
		  v_pos.z = -1.0;
		  v_texture = vTexture;
		  v_texture.y = 1.0 - v_texture.y;
		  gl_Position = vec4(v_pos, 1.0);
		}
	-->
	</source>
</shader>