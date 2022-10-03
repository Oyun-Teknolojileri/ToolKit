<shader>
	<type name = "vertexShader" />
	<uniform name = "ProjectViewModel" />
	<uniform name = "View" />
	<uniform name = "Model" />
	<uniform name = "CamData" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		// Fixed Attributes.
		in vec3 vPosition;
		in vec3 vNormal;
		in vec2 vTexture;

		struct _CamData
		{
			vec3 pos;
			vec3 dir;
			float far;
		};

		uniform _CamData CamData;

		out vec4 v_pos;
		out vec2 v_texture;

		uniform mat4 ProjectViewModel;
		uniform mat4 View;
		uniform mat4 Model;
		uniform float Far;

		void main()
		{
			v_texture = vTexture;
			v_pos = (View * Model * vec4(vPosition, 1.0)) / CamData.far;
		  gl_Position = ProjectViewModel * vec4(vPosition, 1.0);
		}
	-->
	</source>
</shader>