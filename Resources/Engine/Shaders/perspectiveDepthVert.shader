<shader>
	<type name = "vertexShader" />
	<uniform name = "ProjectViewModel" />
		<uniform name = "Model" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		// Fixed Attributes.
		in vec3 vPosition;
		in vec3 vNormal;
		in vec2 vTexture;

		out vec4 v_pos;

		uniform mat4 ProjectViewModel;
		uniform mat4 Model;
		
		void main()
		{
			v_pos = Model * vec4(vPosition, 1.0);
		  gl_Position = ProjectViewModel * vec4(vPosition, 1.0);
		}
	-->
	</source>
</shader>