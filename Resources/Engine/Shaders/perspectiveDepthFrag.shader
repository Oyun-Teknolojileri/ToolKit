<shader>
	<type name = "fragmentShader" />
	<uniform name = "CamData" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		in vec4 v_pos;

		struct _CamData
		{
			vec3 pos;
			vec3 dir;
			float farPlane;
		};

		uniform _CamData CamData;
		uniform float FarPlane;

		void main()
		{
	    float lightDistance = length(v_pos.xyz - CamData.pos);
	    
	    // Convert to [0 1] range
	    lightDistance = lightDistance / CamData.farPlane;
	    
	    gl_FragDepth = lightDistance;
		}
	-->
	</source>
</shader>