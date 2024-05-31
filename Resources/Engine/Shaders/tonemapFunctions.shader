<shader>
	<type name = "includeShader" />
	<source>
	<!--

	#ifndef TONE_MAP_SHADER
	#define TONE_MAP_SHADER

uniform uint useAcesTonemapper;

vec3 ACESFilm(vec3 x)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return clamp( (x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 Tonemap(vec3 color) 
{
	if(useAcesTonemapper > 0u)
  {
		return ACESFilm(color);
	}
	else{
		return (color / (color + vec3(1.0)));
	}
}

#endif

	-->
	</source>
</shader>