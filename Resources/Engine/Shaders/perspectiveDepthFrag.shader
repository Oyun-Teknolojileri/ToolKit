<shader>
	<type name = "fragmentShader" />
	<include name = "VSM.shader" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "ColorAlpha" />
	<uniform name = "alphaMaskTreshold" />
	<define name = "UseAlphaMask" val="0,1" />
	<define name = "EnableDiscardPixel" val="0,1" />
	<source>
	<!--
#version 300 es
precision highp float;

in vec4 v_pos;
in vec2 v_texture;

uniform int DiffuseTextureInUse;
uniform sampler2D s_texture0;
uniform float ColorAlpha;
uniform float alphaMaskTreshold;

out vec4 fragColor;

void main()
{
	float alpha = 1.0;
	if (DiffuseTextureInUse == 1)
	{
		alpha = texture(s_texture0, v_texture).a;
	}
	else
	{
		alpha = ColorAlpha;
	}

	#if EnableDiscardPixel
		#if UseAlphaMask
			if (alpha < alphaMaskTreshold)
			{
				discard;
			}
		#else 
			if (alpha < 0.1)
			{
				discard;
			}
		#endif
	#endif

	vec2 exponents = EvsmExponents;
	vec2 vsmDepth = WarpDepth(length(v_pos.xyz), exponents);

	fragColor = vec4(vsmDepth.xy, vsmDepth.xy * vsmDepth.xy);
}
	-->
	</source>
</shader>