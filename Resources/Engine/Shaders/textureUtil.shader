<shader>
	<type name = "includeShader" />
	<source>
	<!--	

	#ifndef TEXTURE_UTIL_SHADER
	#define TEXTURE_UTIL_SHADER

// TODO there should be a better alogrithm for this
// This shows artifacts in low resolutions (lower than 100x100)
vec2 ClampTextureCoordinates(vec2 coord, vec2 mn, vec2 mx)
{
	return max(min(coord, mx - 0.00015), mn);
}

// Returns uv coordinates and layers such as: vec3(u,v,layer)
// https://kosmonautblog.wordpress.com/2017/03/25/shadow-filtering-for-pointlights/

// Can be improved:
// https://stackoverflow.com/questions/53115467/how-to-implement-texturecube-using-6-sampler2d
vec3 UVWToUVLayer(vec3 vec)
{
	/*
		layer:
			0       1       2       3       4       5
		pos X   neg X   pos Y   neg Y   pos Z   neg Z
	*/
	float layer;
	vec2 coord;
	vec3 absVec = abs(vec);

	if (absVec.x >= absVec.y && absVec.x >= absVec.z)
	{
		if (vec.x > 0.0)
		{
			layer = 0.0;
			vec /= vec.x;
			coord = -vec.zy;
		}
		else
		{
			layer = 1.0;
			vec.y = -vec.y;
			vec /= vec.x;
			coord = -vec.zy;
		}
	}
	else if (absVec.y >= absVec.x && absVec.y >= absVec.z)
	{
		if (vec.y > 0.0)
		{
			layer = 2.0;
			vec /= vec.y;
			coord = vec.xz;
		}
		else
		{
			layer = 3.0;
			vec.x = -vec.x;
			vec /= vec.y;
			coord = vec.xz;
		}
	}
	else
	{
		if (vec.z > 0.0)
		{
			layer = 4.0;
			vec.y = -vec.y;
			vec /= -vec.z;
			coord = -vec.xy;
		}
		else
		{
			layer = 5.0;
			vec /= -vec.z;
			coord = -vec.xy;
		}
	}

	coord = (coord + vec2(1.0)) * 0.5;
	return vec3(coord, layer);
}

#endif

	-->
	</source>
</shader>
