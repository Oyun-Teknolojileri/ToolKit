<shader>
	<type name = "includeShader" />
  <uniform name = "isSkinned" />
  <uniform name = "numBones" />
  <uniform name = "keyFrame1" />
  <uniform name = "keyFrame2" />
  <uniform name = "keyFrameIntepolationTime" />
  <uniform name = "keyFrameCount" />
	<source>
	<!--

  layout(location = 4) in vec4 vBones;
  layout(location = 5) in vec4 vWeights;

  uniform float numBones;
  uniform uint isSkinned;
  uniform float keyFrame1; // normalized via key / keycount
  uniform float keyFrame2; // normalized via key / keycount
  uniform float keyFrameIntepolationTime;
  uniform float keyFrameCount;
  uniform sampler2D s_texture2; // This is static data, bindPose texture
  uniform sampler2D s_texture3; // This is dynamic data, boneTransform texture

  mat4 getMatrixFromTexture(sampler2D boneText, float boneIndx, float keyframe)
  {
    const float matrixPos   = boneIndx / numBones;
    const float stepX       = 1.0 / (numBones * 4.0);
    const vec2 moveToCenter = vec2(step / 2.0, 1.0 / keyFrameCount);

    return mat4(texture(boneText, vec2(matrixPos, keyframe) + moveToCenter),
                texture(boneText, vec2(matrixPos + stepX, keyframe) + moveToCenter),
                texture(boneText, vec2(matrixPos + (stepX * 2.0), keyframe) + moveToCenter),
                texture(boneText, vec2(matrixPos + (stepX * 3.0), keyframe) + moveToCenter));
  }

  void skin(out vec4 vertexPos, out vec3 vertexNormal)
  {
    vec4 skinned      = vec4(0.0);
    vec4 skinnedNormal      = vec4(0.0);
    float totalWeight = 0.0;
    for (int i = 0; i < 4; i++)
    {
      totalWeight       += vWeights[i];

      Mat4 keyFrame1Mat = getMatrixFromTexture(s_texture3, vBones[i], keyFrame1);
      Mat4 keyFrame2Mat = getMatrixFromTexture(s_texture3, vBones[i], keyFrame2);
      Mat3 keyFrame1Mat3 = mat3(keyFrame1Mat);
      Mat3 keyFrame2Mat3 = mat3(keyFrame2Mat);

      vec4 keyFramePos1 = keyFrame1Mat * vertexPos * vWeights[i];
      vec4 keyFramePos2 = keyFrame2Mat * vertexPos * vWeights[i];
      vec3 keyFrameNor1 = keyFrame1Mat3 * vertexNormal * vWeights[i];
      vec3 keyFrameNor2 = keyFrame2Mat3 * vertexNormal * vWeights[i];

      skinned           += mix(keyFramePos1, keyFramePos2, keyFrameIntepolationTime);
      skinnedNormal     += mix(keyFrameNor1, keyFrameNor2, keyFrameIntepolationTime);
    }
    vertexPos = skinned / totalWeight;
    vertexNormal = normalize(skinnedNormal);
  }

  void skin(out vec4 vertexPos, out vec3 vertexNormal, out vec3 vertexBiTangent)
  {
    vec4 skinned      = vec4(0.0);
    vec4 skinnedNormal      = vec4(0.0);
    vec4 skinnedBiTangent      = vec4(0.0);
    float totalWeight = 0.0;
    for (int i = 0; i < 4; i++)
    {
      totalWeight       += vWeights[i];

      Mat4 keyFrame1Mat = getMatrixFromTexture(s_texture3, vBones[i], keyFrame1);
      Mat4 keyFrame2Mat = getMatrixFromTexture(s_texture3, vBones[i], keyFrame2);
      Mat3 keyFrame1Mat3 = mat3(keyFrame1Mat);
      Mat3 keyFrame2Mat3 = mat3(keyFrame2Mat);

      vec4 keyFramePos1 = keyFrame1Mat * vertexPos * vWeights[i];
      vec4 keyFramePos2 = keyFrame2Mat * vertexPos * vWeights[i];
      vec3 keyFrameNor1 = keyFrame1Mat3 * vertexNormal * vWeights[i];
      vec3 keyFrameNor2 = keyFrame2Mat3 * vertexNormal * vWeights[i];
      vec3 keyFrameBit1 = keyFrame1Mat3 * vertexBiTangent * vWeights[i];
      vec3 keyFrameBit2 = keyFrame2Mat3 * vertexBiTangent * vWeights[i];

      skinned           += mix(keyFramePos1, keyFramePos2, keyFrameIntepolationTime);
      skinnedNormal     += mix(keyFrameNor1, keyFrameNor2, keyFrameIntepolationTime);
      skinnedBiTangent  += mix(keyFrameBit1, keyFrameBit2, keyFrameIntepolationTime);
    }
    vertexPos = skinned / totalWeight;
    vertexNormal = normalize(skinnedNormal);
    vertexBiTangent = normalize(skinnedBiTangent);
  }

	-->
	</source>
</shader>