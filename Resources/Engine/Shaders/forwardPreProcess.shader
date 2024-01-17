<shader>
	<type name = "fragmentShader" />
	<uniform name = "useAlphaMask" />
	<uniform name = "alphaMaskTreshold" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "Color" />
	<uniform name = "normalMapInUse" />
	<source>
	<!--
		#version 300 es
		precision highp float;
		precision highp int;

		in vec3 v_viewDepth;
		in vec3 v_normal;
		in vec2 v_texture;
		in mat3 TBN;

		uniform int useAlphaMask;
		uniform float alphaMaskTreshold;
		uniform vec4 Color;
		uniform int normalMapInUse;

		layout (location = 0) out vec3 fragViewDepth;
		layout (location = 1) out vec3 fragNormal;

		uniform int DiffuseTextureInUse;
		uniform sampler2D s_texture0; // color
		uniform sampler2D s_texture9; // normal

		void main()
		{
			vec4 color;
			if (DiffuseTextureInUse == 1)
			{
				color = texture(s_texture0, v_texture).rgba;
			}
			else
			{
				color = Color;
			}

			if (useAlphaMask == 1)
			{
				if (color.a < alphaMaskTreshold)
				{
					discard;
				}
			}

			fragViewDepth = v_viewDepth;

			if (normalMapInUse == 1)
			{
				fragNormal = texture(s_texture9, v_texture).xyz;
				fragNormal = fragNormal * 2.0 - 1.0;
				fragNormal = TBN * fragNormal;
				fragNormal = normalize(fragNormal);
			}
			else
			{
				fragNormal = normalize(v_normal);
			}
		}

	-->
	</source>
</shader>