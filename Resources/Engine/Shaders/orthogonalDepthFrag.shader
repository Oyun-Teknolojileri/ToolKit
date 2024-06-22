<shader>
	<type name = "fragmentShader" />
	<include name = "VSM.shader" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "ColorAlpha" />
	<source>
	<!--
#version 300 es
precision highp float;

in vec2 v_texture;

uniform int DiffuseTextureInUse;
uniform sampler2D s_texture0;
uniform float ColorAlpha;

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

	if (alpha < 0.1)
	{
		discard;
	}

	vec2 exponents = EvsmExponents;
	vec2 vsmDepth = WarpDepth(gl_FragCoord.z, exponents);

	fragColor = vec4(vsmDepth.xy, vsmDepth.xy * vsmDepth.xy).xzxz;
}
	-->
	</source>
</shader>