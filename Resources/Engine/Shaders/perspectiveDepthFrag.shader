<shader>
	<type name = "fragmentShader" />
	<include name = "VSM.shader" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "ColorAlpha" />
	<uniform name = "alphaMaskTreshold" />
	<define name = "DrawAlphaMasked" val="0,1" />
	<define name = "EVSM4" val="0,1" />
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

#if DrawAlphaMasked
	if (alpha <= alphaMaskTreshold)
	{
		discard;
	}
#endif

	vec2 exponents = EvsmExponents;
	vec2 vsmDepth = WarpDepth(length(v_pos.xyz), exponents);

#if EVSM4
		fragColor = vec4(vsmDepth.xy, vsmDepth.xy * vsmDepth.xy);
#else
	fragColor = vec4(vsmDepth.xy, vsmDepth.xy * vsmDepth.xy).xzxz;
#endif
}
	-->
	</source>
</shader>