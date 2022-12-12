<shader>
	<type name = "fragmentShader" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "Color" />
	<uniform name = "useAlphaMask" />
	<uniform name = "alphaMaskTreshold" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		in vec3 v_pos;
		in vec3 v_normal;
		in vec2 v_texture;

		layout (location = 0) out vec3 fragPosition;
		layout (location = 1) out vec3 fragNormal;
		layout (location = 2) out vec3 fragColor;

		uniform int DiffuseTextureInUse;
		uniform sampler2D s_texture0;
		uniform vec4 Color;

		uniform int useAlphaMask;
		uniform float alphaMaskTreshold;

		void main()
		{
			vec4 color = texture(s_texture0, v_texture).rgba * float(DiffuseTextureInUse) + (1.0 - float(DiffuseTextureInUse)) * Color;
			if (useAlphaMask == 1)
			{
				if (color.a < alphaMaskTreshold)
				{
					discard;
				}
			}

		  fragPosition = v_pos;
		  fragNormal = normalize(v_normal);
			fragColor = color.xyz;
		}
	-->
	</source>
</shader>