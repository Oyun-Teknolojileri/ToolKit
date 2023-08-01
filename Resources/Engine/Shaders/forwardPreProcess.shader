<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		in vec3 v_viewDepth;
		in vec3 v_normal;

		layout (location = 0) out vec3 fragViewDepth;
		layout (location = 1) out vec3 fragNormal;

		void main()
		{
			fragViewDepth = v_viewDepth;
			fragNormal    = normalize(v_normal);
		}

	-->
	</source>
</shader>