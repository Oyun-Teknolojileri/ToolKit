<shader>
	<type name = "includeShader" />
	<source>
	<!--
		
#ifndef LIGHT_DATA_SHADER
#define LIGHT_DATA_SHADER

const float LightTypeIndex = 0.0;
const float DirColorIndex = 1.0;
const float DirIntensityIndex = 2.0;
const float DirDirectionIndex = 3.0;

const float PointColorIndex = 1.0;
const float PointIntensityIndex = 2.0;
const float PointPositionIndex = 3.0;
const float PointRadiusIndex = 4.0;

const float SpotColorIndex = 1.0;
const float SpotIntensityIndex = 2.0;
const float SpotDirectionIndex = 4.0;
const float SpotPositionIndex = 3.0;
const float SpotRadiusIndex = 5.0;
const float SpotOuterAngleIndex = 6.0;
const float SpotInnerAngleIndex = 7.0;

vec4 ReadVec(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	vec2 loc = vec2(mod(startingPoint, lightDataTextureWidth), floor(startingPoint / lightDataTextureWidth));
	loc = loc / lightDataTextureWidth;
	
	return texture(data, loc);
}

float ReadFloat(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadVec(data, startingPoint, lightDataTextureWidth).r;
}

int ReadInt(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return int(ReadVec(data, startingPoint, lightDataTextureWidth).r);
}

vec2 ReadVec2(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadVec(data, startingPoint, lightDataTextureWidth).rg;
}

vec3 ReadVec3(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadVec(data, startingPoint, lightDataTextureWidth).rgb;
}

mat4 ReadMat4(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	float unit = 1.0 / lightDataTextureWidth;
	vec2 currentPoint = vec2(mod(startingPoint, lightDataTextureWidth), floor(startingPoint * unit));
	vec2 loc = currentPoint * unit;
	vec4 v1 = texture(data, loc).rgba;

	currentPoint = vec2(startingPoint + 1.0, 0.0);
	currentPoint = vec2(mod(currentPoint.x, lightDataTextureWidth), floor(currentPoint.x * unit));
	loc = currentPoint * unit;
	vec4 v2 = texture(data, loc).rgba;

	currentPoint = vec2(startingPoint + 2.0, 0.0);
	currentPoint = vec2(mod(currentPoint.x, lightDataTextureWidth), floor(currentPoint.x * unit));
	loc = currentPoint * unit;
	vec4 v3 = texture(data, loc).rgba;

	currentPoint = vec2(startingPoint + 3.0, 0.0);
	currentPoint = vec2(mod(currentPoint.x, lightDataTextureWidth), floor(currentPoint.x * unit));
	loc = currentPoint * unit;
	vec4 v4 = texture(data, loc).rgba;

	mat4 mat = mat4(v1, v2, v3, v4);
	return mat;
}

float LightType(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + LightTypeIndex, lightDataTextureWidth);
}

vec3 DirLightColor(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadVec3(data, startingPoint + DirColorIndex, lightDataTextureWidth);
}

vec3 PointLightColor(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadVec3(data, startingPoint + PointColorIndex, lightDataTextureWidth);
}

vec3 SpotLightColor(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadVec3(data, startingPoint + SpotColorIndex, lightDataTextureWidth);
}

float DirLightIntensity(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + DirIntensityIndex, lightDataTextureWidth);
}

float PointLightIntensity(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + PointIntensityIndex, lightDataTextureWidth);
}

float SpotLightIntensity(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + SpotIntensityIndex, lightDataTextureWidth);
}

vec3 DirLightDirection(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadVec3(data, startingPoint + DirDirectionIndex, lightDataTextureWidth);
}

vec3 SpotLightDirection(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadVec3(data, startingPoint + SpotDirectionIndex, lightDataTextureWidth);
}

vec3 PointLightPosition(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadVec3(data, startingPoint + PointPositionIndex, lightDataTextureWidth);
}

vec3 SpotLightPosition(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadVec3(data, startingPoint + SpotPositionIndex, lightDataTextureWidth);
}

float PointLightRadius(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + PointRadiusIndex, lightDataTextureWidth);
}

float SpotLightRadius(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + SpotRadiusIndex, lightDataTextureWidth);
}

float SpotLightOuterAngle(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + SpotOuterAngleIndex, lightDataTextureWidth);
}

float SpotLightInnerAngle(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + SpotInnerAngleIndex, lightDataTextureWidth);
}

const float DirProjViewMatrixIndex = 4.0;
const float DirShadowAtlasCoordIndex = 8.0;
const float DirShadowAtlasLayerIndex = 9.0;
const float DirShadowAtlasResRatioIndex = 10.0;
const float DirSoftShadowsIndex = 11.0;
const float DirPCFSamplesIndex = 12.0;
const float DirPCFRadiusIndex = 13.0;
const float DirLightBleedReducIndex = 14.0;
const float DirShadowBiasIndex = 15.0;

mat4 DirLightProjViewMatrix(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadMat4(data, startingPoint + DirProjViewMatrixIndex, lightDataTextureWidth);
}

vec2 DirLightShadowAtlasCoord(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadVec2(data, startingPoint + DirShadowAtlasCoordIndex, lightDataTextureWidth);
}

float DirLightShadowAtlasLayer(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + DirShadowAtlasLayerIndex, lightDataTextureWidth);
}

float DirLightShadowAtlasResRatio(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + DirShadowAtlasResRatioIndex, lightDataTextureWidth);
}

int DirLightSoftShadows(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadInt(data, startingPoint + DirSoftShadowsIndex, lightDataTextureWidth);
}

int DirLightPCFSamples(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadInt(data, startingPoint + DirPCFSamplesIndex, lightDataTextureWidth);
}

float DirLightPCFRadius(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + DirPCFRadiusIndex, lightDataTextureWidth);
}

float DirLightBleedReduction(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + DirLightBleedReducIndex, lightDataTextureWidth);
}

float DirLightShadowBias(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + DirShadowBiasIndex, lightDataTextureWidth);
}

const float SpotProjViewMatrixIndex = 8.0;
const float SpotShadowCameraFarIndex = 12.0;
const float SpotShadowAtlasCoordIndex = 13.0;
const float SpotShadowAtlasLayerIndex = 14.0;
const float SpotShadowAtlasResRatioIndex = 15.0;
const float SpotSoftShadowsIndex = 16.0;
const float SpotPCFSamplesIndex = 17.0;
const float SpotPCFRadiusIndex = 18.0;
const float SpotLightBleedReducIndex = 19.0;
const float SpotShadowBiasIndex = 20.0;

mat4 SpotLightProjViewMatrix(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadMat4(data, startingPoint + SpotProjViewMatrixIndex, lightDataTextureWidth);
}

float SpotLightShadowCameraFar(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + SpotShadowCameraFarIndex, lightDataTextureWidth);
}

vec2 SpotLightShadowAtlasCoord(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadVec2(data, startingPoint + SpotShadowAtlasCoordIndex, lightDataTextureWidth);
}

float SpotLightShadowAtlasLayer(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + SpotShadowAtlasLayerIndex, lightDataTextureWidth);
}

float SpotLightShadowAtlasResRatio(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + SpotShadowAtlasResRatioIndex, lightDataTextureWidth);
}

int SpotLightSoftShadows(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadInt(data, startingPoint + SpotSoftShadowsIndex, lightDataTextureWidth);
}

int SpotLightPCFSamples(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadInt(data, startingPoint + SpotPCFSamplesIndex, lightDataTextureWidth);
}

float SpotLightPCFRadius(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + SpotPCFRadiusIndex, lightDataTextureWidth);
}

float SpotLightBleedReduction(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + SpotLightBleedReducIndex, lightDataTextureWidth);
}

float SpotLightShadowBias(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + SpotShadowBiasIndex, lightDataTextureWidth);
}

const float PointShadowCameraFarIndex = 5.0;
const float PointShadowAtlasCoordIndex = 6.0;
const float PointShadowAtlasLayerIndex = 7.0;
const float PointShadowAtlasResRatioIndex = 8.0;
const float PointSoftShadowsIndex = 9.0;
const float PointPCFSamplesIndex = 10.0;
const float PointPCFRadiusIndex = 11.0;
const float PointLightBleedReducIndex = 12.0;
const float PointShadowBiasIndex = 13.0;

float PointLightShadowCameraFar(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + PointShadowCameraFarIndex, lightDataTextureWidth);
}

vec2 PointLightShadowAtlasCoord(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadVec2(data, startingPoint + PointShadowAtlasCoordIndex, lightDataTextureWidth);
}

float PointLightShadowAtlasLayer(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + PointShadowAtlasLayerIndex, lightDataTextureWidth);
}

float PointLightShadowAtlasResRatio(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + PointShadowAtlasResRatioIndex, lightDataTextureWidth);
}

int PointLightSoftShadows(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadInt(data, startingPoint + PointSoftShadowsIndex, lightDataTextureWidth);
}

int PointLightPCFSamples(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadInt(data, startingPoint + PointPCFSamplesIndex, lightDataTextureWidth);
}

float PointLightPCFRadius(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + PointPCFRadiusIndex, lightDataTextureWidth);
}

float PointLightBleedReduction(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + PointLightBleedReducIndex, lightDataTextureWidth);
}

float PointLightShadowBias(sampler2D data, float startingPoint, float lightDataTextureWidth)
{
	return ReadFloat(data, startingPoint + PointShadowBiasIndex, lightDataTextureWidth);
}

#endif

	-->
	</source>
</shader>