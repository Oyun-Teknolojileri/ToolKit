<shader>
	<type name = "fragmentShader" />
	<uniform name = "Color" />
	<uniform name = "useAlphaMask" />
	<uniform name = "alphaMaskTreshold" />
	<source>
	<!--
		#version 300 es
		precision mediump float;
		
		uniform int useAlphaMask;
		uniform float alphaMaskTreshold;
		uniform vec4 Color;
		out vec4 fragColor;

		void main()
		{
			if (useAlphaMask == 1)
			{
				if (Color.a < alphaMaskTreshold)
				{
					discard;
				}
			}

		  fragColor = Color;
		}
	-->
	</source>
</shader>