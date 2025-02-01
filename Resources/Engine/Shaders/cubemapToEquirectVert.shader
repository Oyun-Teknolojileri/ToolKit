<shader>
	<type name = "vertexShader" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		layout (location = 0) in vec2 vPosition;

		out vec2 uv;

		void main()
		{
			uv = vPosition * 0.5 + 0.5; // Map from [-1,1] to [0,1]
			gl_Position = vec4(vPosition, 0.0, 1.0);
		}
	-->
	</source>
</shader>