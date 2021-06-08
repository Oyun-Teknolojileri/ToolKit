<shader>
	<type name = "vertexShader" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		// Fixed Attributes.
		in vec3 vPosition;
		in vec3 vNormal;
		in vec2 vTexture;

		out vec3 v_pos;
		out vec3 v_normal;
		out vec2 v_texture;
		
		void main()
		{
		  v_texture = vTexture;
		  v_normal = vNormal;
		  gl_Position = vec4(vPosition.xy * 2.0, -1.0, 1.0);
		}
	-->
	</source>
</shader>