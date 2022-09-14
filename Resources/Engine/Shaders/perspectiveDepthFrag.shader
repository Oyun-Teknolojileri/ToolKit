<shader>
	<type name = "fragmentShader" />
	<uniform name = "CamData" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "ColorAlpha" />
	<source>
	<!--
		#version 300 es
		precision mediump float;

		in vec4 v_pos;
		in vec3 v_normal;
		in vec2 v_texture;

		struct _CamData
		{
			vec3 pos;
			vec3 dir;
			float farPlane;
		};

		uniform _CamData CamData;
		uniform float FarPlane;

		uniform int diffuseTextureInUse;
		uniform sampler2D s_texture0;
		uniform float colorAlpha;

		void main()
		{
			float alpha = float(1 - diffuseTextureInUse) * colorAlpha
				+ float(diffuseTextureInUse) * texture(s_texture0, v_texture).a;
			if (alpha < 0.1)
			{
				discard;
			}

	    float lightDistance = length(v_pos.xyz - CamData.pos);
	    
	    // Convert to [0 1] range
	    lightDistance = lightDistance / CamData.farPlane;
	    
	    gl_FragDepth = lightDistance;
		}
	-->
	</source>
</shader>