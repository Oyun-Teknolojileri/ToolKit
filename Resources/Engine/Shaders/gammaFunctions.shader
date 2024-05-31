<shader>
	<type name = "includeShader" />
	<source>
	<!--

#ifndef GAMMA_SHADER
#define GAMMA_SHADER

uniform float gamma;

vec3 Gamma(vec3 color)
{
	return pow(color, vec3(1.0/gamma));
}

#endif

	-->
	</source>
</shader>