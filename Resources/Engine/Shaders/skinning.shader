<shader>
	<type name = "includeShader" />
  <uniform name = "isSkinned" />
  <uniform name = "numBones" />
  <uniform name = "keyFrame1" />
  <uniform name = "keyFrame2" />
  <uniform name = "keyFrameIntepolationTime" />
  <uniform name = "keyFrameCount" />
  <uniform name  = "isAnimated" />
  <uniform name = "blendAnimation" />
  <uniform name = "blendFactor" />
  <uniform name = "blendKeyFrame1" />
  <uniform name = "blendKeyFrame2" />
  <uniform name = "blendKeyFrameIntepolationTime" />
  <uniform name = "blendKeyFrameCount" />
	<source>
	<!--

#ifndef SKIN_SHADER
#define SKIN_SHADER

layout(location = 4) in vec4 vBones;
layout(location = 5) in vec4 vWeights;

uniform float numBones;
uniform uint isSkinned;
uniform uint isAnimated;
uniform float keyFrame1; // normalized via key / keycount
uniform float keyFrame2; // normalized via key / keycount
uniform float keyFrameIntepolationTime;
uniform float keyFrameCount;
uniform int blendAnimation;
uniform float blendFactor;
uniform float blendKeyFrame1;
uniform float blendKeyFrame2;
uniform float blendKeyFrameIntepolationTime;
uniform float blendKeyFrameCount;
uniform sampler2D s_texture2; // Blend animation data texture
uniform sampler2D s_texture3; // Animation data texture

mat4 getMatrixFromTexture(sampler2D animDataTexture, float boneIndx, float keyframe, float numberOfKeyFrames)
{
  float matrixPos   = boneIndx / numBones;
  float stepX       = 1.0 / (numBones * 4.0);
  vec2 moveToCenter = vec2(stepX / 2.0, 1.0 / (numberOfKeyFrames * 2.0));

  return mat4(texture(animDataTexture, vec2(matrixPos, keyframe) + moveToCenter),
              texture(animDataTexture, vec2(matrixPos + stepX, keyframe) + moveToCenter),
              texture(animDataTexture, vec2(matrixPos + (stepX * 2.0), keyframe) + moveToCenter),
              texture(animDataTexture, vec2(matrixPos + (stepX * 3.0), keyframe) + moveToCenter));
}

// keyFramesData-> x: keyFrame1 y: keyFrame2 z: time w: key frame count
void skinCalc(in sampler2D animDataTexture, vec4 keyFramesData, in vec4 vertexPos, out vec4 skinnedPos)
{
  skinnedPos      = vec4(0.0);
  float totalWeight = 0.0;
  for (int i = 0; i < 4; i++)
  {
    totalWeight       += vWeights[i];

    if (isAnimated == 0u)
    {
      mat4 bindPoseMatrix = getMatrixFromTexture(animDataTexture, vBones[i], 0.5, keyFramesData.w);
      skinnedPos += bindPoseMatrix * vertexPos * vWeights[i];
    }
    else
    {
      mat4 keyFrame1Mat = getMatrixFromTexture(animDataTexture, vBones[i], keyFramesData.x, keyFramesData.w);
      mat4 keyFrame2Mat = getMatrixFromTexture(animDataTexture, vBones[i], keyFramesData.y, keyFramesData.w);

      vec4 keyFramePos1 = keyFrame1Mat * vertexPos * vWeights[i];
      vec4 keyFramePos2 = keyFrame2Mat * vertexPos * vWeights[i];

      skinnedPos           += mix(keyFramePos1, keyFramePos2, keyFramesData.z);
    }
  }
  skinnedPos = skinnedPos / totalWeight;
}

// keyFramesData-> x: keyFrame1 y: keyFrame2 z: time w: key frame count
void skinCalc
(
  in sampler2D animDataTexture,
  vec4 keyFramesData,
  in vec4 vertexPos,
  in vec3 vertexNormal,
  out vec4 skinnedPos,
  out vec3 skinnedNormal
)
{
  skinnedPos      = vec4(0.0);
  skinnedNormal   = vec3(0.0);
  float totalWeight = 0.0;
  for (int i = 0; i < 4; i++)
  {
    totalWeight       += vWeights[i];

    if (isAnimated == 0u)
    {
      mat4 bindPoseMatrix = getMatrixFromTexture(animDataTexture, vBones[i], 0.5, keyFramesData.w);
      skinnedPos += bindPoseMatrix * vertexPos * vWeights[i];
      skinnedNormal += mat3(bindPoseMatrix) * vertexNormal * vWeights[i];
    }
    else
    {
      mat4 keyFrame1Mat = getMatrixFromTexture(animDataTexture, vBones[i], keyFramesData.x, keyFramesData.w);
      mat4 keyFrame2Mat = getMatrixFromTexture(animDataTexture, vBones[i], keyFramesData.y, keyFramesData.w);
      mat3 keyFrame1Mat3 = mat3(keyFrame1Mat);
      mat3 keyFrame2Mat3 = mat3(keyFrame2Mat);

      vec4 keyFramePos1 = keyFrame1Mat * vertexPos * vWeights[i];
      vec4 keyFramePos2 = keyFrame2Mat * vertexPos * vWeights[i];
      vec3 keyFrameNor1 = keyFrame1Mat3 * vertexNormal * vWeights[i];
      vec3 keyFrameNor2 = keyFrame2Mat3 * vertexNormal * vWeights[i];

      skinnedPos        += mix(keyFramePos1, keyFramePos2, keyFramesData.z);
      skinnedNormal     += mix(keyFrameNor1, keyFrameNor2, keyFramesData.z);
    }
  }
  skinnedPos = skinnedPos / totalWeight;
  skinnedNormal = normalize(skinnedNormal);
}

// keyFramesData-> x: keyFrame1 y: keyFrame2 z: time w: key frame count
void skinCalc
(
  in sampler2D animDataTexture,
  vec4 keyFramesData,
  in vec4 vertexPos,
  in vec3 vertexNormal,
  in vec3 vertexBiTangent,
  out vec4 skinnedPos,
  out vec3 skinnedNormal,
  out vec3 skinnedBiTangent
)
{
  skinnedPos        = vec4(0.0);
  skinnedNormal     = vec3(0.0);
  skinnedBiTangent  = vec3(0.0);
  float totalWeight = 0.0;
  for (int i = 0; i < 4; i++)
  {
    totalWeight       += vWeights[i];

    if (isAnimated == 0u)
    {
      mat4 bindPoseMatrix = getMatrixFromTexture(animDataTexture, vBones[i], 0.5, keyFramesData.w);
      mat3 bindPoseMatrix3x3 = mat3(bindPoseMatrix);
      skinnedPos += bindPoseMatrix * vertexPos * vWeights[i];
      skinnedNormal += bindPoseMatrix3x3 * vertexNormal * vWeights[i];
      skinnedBiTangent += bindPoseMatrix3x3 * vertexBiTangent * vWeights[i];
    }
    else
    {
      mat4 keyFrame1Mat = getMatrixFromTexture(animDataTexture, vBones[i], keyFramesData.x, keyFramesData.w);
      mat4 keyFrame2Mat = getMatrixFromTexture(animDataTexture, vBones[i], keyFramesData.y, keyFramesData.w);
      mat3 keyFrame1Mat3 = mat3(keyFrame1Mat);
      mat3 keyFrame2Mat3 = mat3(keyFrame2Mat);

      vec4 keyFramePos1 = keyFrame1Mat * vertexPos * vWeights[i];
      vec4 keyFramePos2 = keyFrame2Mat * vertexPos * vWeights[i];
      vec3 keyFrameNor1 = keyFrame1Mat3 * vertexNormal * vWeights[i];
      vec3 keyFrameNor2 = keyFrame2Mat3 * vertexNormal * vWeights[i];
      vec3 keyFrameBit1 = keyFrame1Mat3 * vertexBiTangent * vWeights[i];
      vec3 keyFrameBit2 = keyFrame2Mat3 * vertexBiTangent * vWeights[i];

      skinnedPos        += mix(keyFramePos1, keyFramePos2, keyFramesData.z);
      skinnedNormal     += mix(keyFrameNor1, keyFrameNor2, keyFramesData.z);
      skinnedBiTangent  += mix(keyFrameBit1, keyFrameBit2, keyFramesData.z);
    }
  }
  skinnedPos = skinnedPos / totalWeight;
  skinnedNormal = normalize(skinnedNormal);
  skinnedBiTangent = normalize(skinnedBiTangent);
}

void skin(in vec4 vertexPos, out vec4 skinnedPos)
{
  skinCalc(s_texture3, vec4(keyFrame1, keyFrame2, keyFrameIntepolationTime, keyFrameCount), vertexPos, skinnedPos);

  if (blendAnimation != 0)
  {
    vec4 blendingAnimSkinnedPos;
    skinCalc
    (
      s_texture2,
      vec4(blendKeyFrame1, blendKeyFrame2, blendKeyFrameIntepolationTime, blendKeyFrameCount),
      vertexPos,
      blendingAnimSkinnedPos
    );
    skinnedPos = mix(skinnedPos, blendingAnimSkinnedPos, blendFactor);
  }
}

void skin(in vec4 vertexPos, in vec3 vertexNormal, out vec4 skinnedPos, out vec3 skinnedNormal)
{
  skinCalc
  (
    s_texture3,
    vec4(keyFrame1, keyFrame2, keyFrameIntepolationTime, keyFrameCount),
    vertexPos,
    vertexNormal,
    skinnedPos,
    skinnedNormal
  );

  if (blendAnimation != 0)
  {
    vec4 blendingAnimSkinnedPos;
    vec3 blendingAnimNormal;
    skinCalc
    (
      s_texture2,
      vec4(blendKeyFrame1, blendKeyFrame2, blendKeyFrameIntepolationTime, blendKeyFrameCount),
      vertexPos,
      vertexNormal,
      blendingAnimSkinnedPos,
      blendingAnimNormal
    );
    skinnedPos = mix(skinnedPos, blendingAnimSkinnedPos, blendFactor);
    skinnedNormal = normalize(mix(skinnedNormal, blendingAnimNormal, blendFactor));
  }
}

void skin
(
  in vec4 vertexPos,
  in vec3 vertexNormal,
  in vec3 vertexBiTangent,
  out vec4 skinnedPos,
  out vec3 skinnedNormal,
  out vec3 skinnedBiTangent
)
{
  skinCalc
  (
    s_texture3,
    vec4(keyFrame1, keyFrame2, keyFrameIntepolationTime, keyFrameCount),
    vertexPos,
    vertexNormal,
    vertexBiTangent,
    skinnedPos,
    skinnedNormal,
    skinnedBiTangent
  );

  if (blendAnimation != 0)
  {
    vec4 blendingAnimSkinnedPos;
    vec3 blendingAnimNormal;
    vec3 blendingBiTangent;
    skinCalc(s_texture2, vec4(blendKeyFrame1, blendKeyFrame2, blendKeyFrameIntepolationTime, blendKeyFrameCount),
      vertexPos, vertexNormal, vertexBiTangent, blendingAnimSkinnedPos, blendingAnimNormal, blendingBiTangent);
    skinnedPos = mix(skinnedPos, blendingAnimSkinnedPos, blendFactor);
    skinnedNormal = normalize(mix(skinnedNormal, blendingAnimNormal, blendFactor));
    skinnedBiTangent = normalize(mix(skinnedBiTangent, blendingBiTangent, blendFactor));
  }
}

#endif

	-->
	</source>
</shader>
