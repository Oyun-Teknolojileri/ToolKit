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
		in vec2 v_texture;

		struct _CamData
		{
			vec3 pos;
			vec3 dir;
		};

		uniform _CamData CamData;

		uniform int diffuseTextureInUse;
		uniform sampler2D s_texture0;
		uniform float colorAlpha;

		out vec4 fragColor;

		vec2 ComputeMoments(float depth)
		{
			vec2 moments;
			moments.x = depth;
			float dx = dFdx(moments.x);
			float dy = dFdy(moments.x);
			moments.y = moments.x * moments.x * 0.25 * (dx * dx + dy * dy);
			return moments;
		}

		void main()
		{
			float alpha = float(1 - diffuseTextureInUse) * colorAlpha
				+ float(diffuseTextureInUse) * texture(s_texture0, v_texture).a;
			if (alpha < 0.1)
			{
				discard;
			}

	    vec2 lightDistance = ComputeMoments(length(v_pos.xyz - CamData.pos));
	    fragColor = vec4(lightDistance, 0.0, 0.0);
		}
	-->
	</source>
</shader>