<shader>
	<type name = "fragmentShader" />\
	<uniform name = "Model" />
	<source>
	<!--
		#version 300 es
		precision highp float;
		
		uniform mat4 Model;
		in vec3 v_normal;
		out vec4 fragColor;		
		
		void main()
		{
			vec3 n = (transpose(Model)* vec4(v_normal, 1.0)).xyz * 0.8;  
			for (int i = 0; i < 3; i++)
			{
				if (n[i] < 0.0)
				{
					n[i] *= -0.3;
				}
			}
			fragColor = vec4(n, 1.0);
		}
	-->
	</source>
</shader>