<shader>
	<type name = "fragmentShader" />
	<uniform name = "useAlphaMask" />
	<uniform name = "alphaMaskTreshold" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "Color" />
	<source>
	<!--
		#version 300 es
		precision mediump float;

		uniform vec4 Color;
		uniform int DiffuseTextureInUse;
		uniform sampler2D s_texture0;
		uniform int useAlphaMask;
		uniform float alphaMaskTreshold;

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
				if(color.a < alphaMaskTreshold){
					discard;
				}
			}

			fragColor = color;
		}
	-->
	</source>
</shader>