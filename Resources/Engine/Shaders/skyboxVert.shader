<shader>
	<type name = "vertexShader" />
	<uniform name = "ProjectionViewNoTr" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		// Fixed Attributes.
		layout (location = 0) in vec3 vPosition;
		layout (location = 1) in vec3 vNormal;
		layout (location = 2) in vec2 vTexture;

		uniform mat4 ProjectionViewNoTr;

		out vec3 v_pos;

		void main()
		{
			v_pos = vPosition;

			vec4 clipPos = ProjectionViewNoTr * vec4(vPosition, 1.0);

			// Make z equals w. So after perspective division the depth
			// value will always 1(maximum depth value).
			gl_Position = clipPos.xyww;
		}
	-->
	</source>
</shader>