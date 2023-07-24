<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		in vec3 v_linearDepth;
		in vec3 v_normal;

		layout (location = 0) out vec3 fragLinearDepth;
		layout (location = 1) out vec3 fragNormal;

		void main()
		{
			fragLinearDepth = v_linearDepth;
			fragNormal      = normalize(v_normal);
		}

	-->
	</source>
</shader>