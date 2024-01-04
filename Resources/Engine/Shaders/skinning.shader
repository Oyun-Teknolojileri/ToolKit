<shader>
    <type name = "includeShader" />
    <uniform name = "isSkinned" />
    <uniform name = "numBones" />
	<source>
	<!--
      layout(location = 4) in vec4 vBones;
      layout(location = 5) in vec4 vWeights;

      uniform uint numBones;
      uniform uint isSkinned;
      uniform sampler2D s_texture2;  // This is static data, bindPose texture
      uniform sampler2D s_texture3; // This is dynamic data, boneTransform texture

      mat4 getMatrixFromTexture(sampler2D boneText, float boneIndx) {
	     float numBonF = float(numBones);
         float v = boneIndx / numBonF;
         float step = 1.0f / (numBonF * 4.0f);
         return mat4
         (
            texture(boneText, vec2(v + (step / 2.0f), 0.0f)),
            texture(boneText, vec2(v + step + (step / 2.0f), 0.0f)),
            texture(boneText, vec2(v + (step * 2.0f) + (step / 2.0f), 0.0f)),
            texture(boneText, vec2(v + (step * 3.0f) + (step / 2.0f), 0.0f))
         );
      }
      vec4 skin(vec4 vertexPos){
         vec4 skinned = vec4(0);
		 float totalWeight = 0.0;
         for(int i = 0; i < 4; i++){
		     totalWeight += vWeights[i];
			 skinned += getMatrixFromTexture(s_texture3, vBones[i]) * getMatrixFromTexture(s_texture2, vBones[i]) * vertexPos * vWeights[i];
         }
         return skinned / totalWeight;
      }
      vec3 skinNormal(vec3 vertexNormal){
         vec3 skinned = vec3(0);
         for(int i = 0; i < 4; i++){
            skinned += mat3(getMatrixFromTexture(s_texture3, vBones[i])) * mat3(getMatrixFromTexture(s_texture2, vBones[i])) * vertexNormal * vWeights[i];
         }
         return normalize(skinned);
      }
	-->
	</source>
</shader>