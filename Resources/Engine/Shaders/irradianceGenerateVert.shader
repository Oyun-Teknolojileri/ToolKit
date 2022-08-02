<shader>
	<type name = "vertexShader" />
	<uniform name = "ProjectViewModel" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		layout (location = 0) in vec3 vPosition;
		layout (location = 1) in vec3 vNormal;
		layout (location = 2) in vec2 vTexture;

		out vec3 v_pos;

		uniform mat4 ProjectViewModel;

		void main()
		{
		    v_pos = vPosition;  
		    gl_Position =  ProjectViewModel * vec4(v_pos, 1.0);
		}
	-->
	</source>
</shader>