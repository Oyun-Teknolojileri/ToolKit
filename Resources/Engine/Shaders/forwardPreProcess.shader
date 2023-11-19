<shader>
	<type name = "fragmentShader" />
	<uniform name = "useAlphaMask" />
	<uniform name = "alphaMaskTreshold" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "Color" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		in vec3 v_viewDepth;
		in vec3 v_normal;
		in vec2 v_texture;

		uniform int useAlphaMask;
		uniform float alphaMaskTreshold;
		uniform vec4 Color;

		layout (location = 0) out vec3 fragViewDepth;
		layout (location = 1) out vec3 fragNormal;

		uniform int DiffuseTextureInUse;
		uniform sampler2D s_texture0; // color

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
			fragNormal    = normalize(v_normal);
		}

	-->
	</source>
</shader>