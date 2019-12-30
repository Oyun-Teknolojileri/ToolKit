<shader>
	<type name = "vertexShader" />
	<uniform name = "ProjectViewModel" />
	<uniform name = "InverseTransModel" />
	<source>
		#version 300 es
		
		// Fixed Attributes. Never changes !
		in vec3 vPosition;
		in vec3 vNormal;
		in vec2 vTexture;
		in vec3 vBiTan;
		
		// Declare used variables in the xml unifrom attribute.
		uniform mat4 ProjectViewModel;
		uniform mat4 InverseTransModel;
		
		out vec3 v_normal;
		out vec2 v_texCoord;
		out vec3 v_bitan;
		void main()
		{
		   gl_Position = ProjectViewModel * vec4(vPosition, 1.0);
		   v_texCoord = vTexture;
		   v_normal = vNormal;
		   v_bitan = (InverseTransModel * vec4(vBiTan, 1.0)).xyz;
		}
	</source>
</shader>