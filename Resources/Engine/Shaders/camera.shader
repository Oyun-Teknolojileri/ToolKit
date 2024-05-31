<shader>
	<type name = "includeShader" />
	<uniform name = "CamData.pos" />
	<uniform name = "CamData.dir" />
	<uniform name = "CamData.far" />
	<source>
	<!--

#ifndef CAMERA_SHADER
#define CAMERA_SHADER

struct _CamData
{
	vec3 pos;
	vec3 dir;
	float far;
};

uniform _CamData CamData;

#endif

	-->
	</source>
</shader>