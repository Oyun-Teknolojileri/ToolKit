<shader>
	<type name = "fragmentShader" />
	<uniform name = "Color" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "useAlphaMask" />
	<uniform name = "alphaMaskTreshold" />
	<source>
	<!--
		#version 300 es
		precision mediump float;
		
		uniform int useAlphaMask;
		uniform float alphaMaskTreshold;
		uniform vec4 Color;
		uniform int DiffuseTextureInUse;
		uniform sampler2D s_texture0;

		in vec2 v_texture;

		out vec4 fragColor;

		void main()
		{
			vec4 color;
			if (DiffuseTextureInUse == 1)
			{
				color = texture(s_texture0, v_texture);
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

		  fragColor = color;
		}
	-->
	</source>
</shader>