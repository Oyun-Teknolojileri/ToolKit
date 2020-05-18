<shader>
	<type name = "vertexShader" />
	<uniform name = "ProjectViewModel" />
	<source>
	<!--
		#version 300 es
		
		in vec3 vPosition;
		in vec3 vNormal;
		in vec2 vTexture;
		in vec3 vBiTan;
		
		uniform mat4 ProjectViewModel;
		
		out vec3 v_normal;
		out vec2 v_texCoord;
		void main()
		{
		   gl_Position = ProjectViewModel * vec4(vPosition, 1.0);
		   v_texCoord = vTexture;
		   v_normal = vNormal;
		}
	-->
	</source>
</shader>