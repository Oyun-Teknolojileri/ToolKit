<shader>
	<type name = "vertexShader" />
	<uniform name = "ProjectViewModel" />
	<uniform name = "CamData" />
	<uniform name = "Model" />
	<uniform name = "InverseTransModel" />
	<uniform name = "GridData" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		// Fixed Attributes.
		in vec3 vPosition;

		struct _CamData
		{
			vec3 pos;
			vec3 dir;
		};
		uniform _CamData CamData;

		struct _GridData
		{
			float cellSize;
			float gridSize;
			vec3 horizontalAxisColor;
			vec3 verticalAxisColor;
		};
		uniform _GridData GridData;

		// Declare used variables in the xml unifrom attribute.
		uniform mat4 ProjectViewModel;
		uniform mat4 InverseTransModel;
		uniform mat4 Model;

		out vec2 o_gridPos;
		out vec2 o_cameraGridPos;
		out vec3 o_viewDir;
		
		void main()
		{
			o_gridPos = vec2(vPosition.x, vPosition.y);

			vec3 cameraGridPos = (InverseTransModel * vec4(CamData.pos, 1.0)).xyz;
			o_cameraGridPos = cameraGridPos.xz;

			vec4 v = vec4(o_gridPos.x, 0, o_gridPos.y, 1);
			v.y -= cameraGridPos.y;
			v = Model * v;
			o_viewDir = -v.xyz;

			gl_Position = ProjectViewModel * vec4(vPosition, 1.0);
		}
	-->
	</source>
</shader>