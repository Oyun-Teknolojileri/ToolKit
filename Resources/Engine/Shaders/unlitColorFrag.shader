<shader>
	<type name = "fragmentShader" />
	<uniform name = "Color" />
	<uniform name = "useAlphaMask" />
	<uniform name = "alphaMask" />
	<source>
	<!--
		#version 300 es
		precision mediump float;
		
		uniform int useAlphaMask;
		uniform float alphaMask;
		uniform vec4 Color;
		out vec4 fragColor;

		void main()
		{
			if (useAlphaMask == 1)
			{
				if (Color.a < alphaMask)
				{
					discard;
				}
			}

		  fragColor = Color;
		}
	-->
	</source>
</shader>