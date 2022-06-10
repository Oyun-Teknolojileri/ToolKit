<shader>
	<type name = "vertexShader" />
	<uniform name = "ProjectViewModel" />
	<uniform name = "InverseTransModel" />
	<uniform name = "Model" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		// Fixed Attributes.
		in vec3 vPosition;
		in vec3 vNormal;
		in vec2 vTexture;

		// Declare used variables in the xml unifrom attribute.
		uniform mat4 ProjectViewModel;
		uniform mat4 InverseTransModel;
		uniform mat4 Model;

		out vec3 v_pos;
		out vec3 v_normal;
		out vec2 v_texture;
		
		void main()
		{
		  v_pos = (Model * vec4(vPosition, 1.0)).xyz;
		  v_normal = (InverseTransModel * vec4(vNormal, 1.0)).xyz;
		  v_texture = vTexture;

		  gl_Position = ProjectViewModel * vec4(vPosition, 1.0);
		}
	-->
	</source>
</shader>