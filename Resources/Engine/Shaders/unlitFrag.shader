<shader>
	<type name = "fragmentShader" />
	<uniform name = "useAlphaMask" />
	<uniform name = "alphaMaskTreshold" />
	<source>
	<!--
		#version 300 es
		precision mediump float;

		uniform sampler2D s_texture0;
		uniform int useAlphaMask;
		uniform float alphaMaskTreshold;

		in vec2 v_texture;
		out vec4 fragColor;

		void main()
		{
		  fragColor = texture(s_texture0, v_texture);
			if (useAlphaMask == 1)
			{
				if(fragColor.a < alphaMaskTreshold){
					discard;
				}
			}
		}
	-->
	</source>
</shader>