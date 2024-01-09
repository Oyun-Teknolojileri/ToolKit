<shader>
	<type name = "includeShader" />
  <uniform name = "isSkinned" />
  <uniform name = "numBones" />
  <uniform name = "framekey1" />
  <uniform name = "framekey2" />
  <uniform name = "framekeyIntepolationTime" />
  <uniform name = "keyframeCount" />
	<source>
	<!--

  layout(location = 4) in vec4 vBones;
  layout(location = 5) in vec4 vWeights;

  uniform float numBones;
  uniform uint isSkinned;
  uniform float framekey1; // normalized via key / keycount
  uniform float framekey2; // normalized via key / keycount
  uniform float framekeyIntepolationTime;
  uniform uint keyframeCount;
  uniform sampler2D s_texture2; // This is static data, bindPose texture
  uniform sampler2D s_texture3; // This is dynamic data, boneTransform texture

  mat4 getMatrixFromTexture(sampler2D boneText, float boneIndx, float keyframe)
  {
    const float matrixPos   = boneIndx / numBones;
    const float stepX       = 1.0 / (numBones * 4.0);
    const vec2 moveToCenter = vec2(step / 2.0, 1.0 / keyframeCount);

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

      Mat4 keyframe1Mat = getMatrixFromTexture(s_texture3, vBones[i], framekey1);
      Mat4 keyframe2Mat = getMatrixFromTexture(s_texture3, vBones[i], framekey2);
      Mat3 keyframe1Mat3 = mat3(keyframe1Mat);
      Mat3 keyframe2Mat3 = mat3(keyframe2Mat);

      vec4 keyframePos1 = keyframe1Mat * vertexPos * vWeights[i];
      vec4 keyframePos2 = keyframe2Mat * vertexPos * vWeights[i];
      vec3 keyframeNor1 = keyframe1Mat3 * vertexNormal * vWeights[i];
      vec3 keyframeNor2 = keyframe2Mat3 * vertexNormal * vWeights[i];

      skinned           += mix(keyframePos1, keyframePos2, framekeyIntepolationTime);
      skinnedNormal     += mix(keyframeNor1, keyframeNor2, framekeyIntepolationTime);
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

      Mat4 keyframe1Mat = getMatrixFromTexture(s_texture3, vBones[i], framekey1);
      Mat4 keyframe2Mat = getMatrixFromTexture(s_texture3, vBones[i], framekey2);
      Mat3 keyframe1Mat3 = mat3(keyframe1Mat);
      Mat3 keyframe2Mat3 = mat3(keyframe2Mat);

      vec4 keyframePos1 = keyframe1Mat * vertexPos * vWeights[i];
      vec4 keyframePos2 = keyframe2Mat * vertexPos * vWeights[i];
      vec3 keyframeNor1 = keyframe1Mat3 * vertexNormal * vWeights[i];
      vec3 keyframeNor2 = keyframe2Mat3 * vertexNormal * vWeights[i];
      vec3 keyframeBit1 = keyframe1Mat3 * vertexBiTangent * vWeights[i];
      vec3 keyframeBit2 = keyframe2Mat3 * vertexBiTangent * vWeights[i];

      skinned           += mix(keyframePos1, keyframePos2, framekeyIntepolationTime);
      skinnedNormal     += mix(keyframeNor1, keyframeNor2, framekeyIntepolationTime);
      skinnedBiTangent  += mix(keyframeBit1, keyframeBit2, framekeyIntepolationTime);
    }
    vertexPos = skinned / totalWeight;
    vertexNormal = normalize(skinnedNormal);
    vertexBiTangent = normalize(skinnedBiTangent);
  }

  vec3 skinNormal(vec3 vertexNormal)
  {
    vec3 skinned = vec3(0);
    for (int i = 0; i < 4; i++)
    {
      skinned += mat3(getMatrixFromTexture(s_texture3, vBones[i])) * vertexNormal * vWeights[i];
    }
    return normalize(skinned);
  }

	-->
	</source>
</shader>