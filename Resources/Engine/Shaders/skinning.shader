<shader>
	<type name = "includeShader" />
	<source>
	<!--
      layout(location = 4) in uvec4 vBones;
      layout(location = 5) in vec4 vWeights;

      uniform float numBones;
      uniform sampler2D s_texture2;  // This is static data, bindPose texture
      uniform sampler2D s_texture3; // This is dynamic data, boneTransform texture

      mat4 getMatrixFromTexture(sampler2D boneText, uint boneIndx) {
         float v = (float(boneIndx) / numBones);
         float step = 1.0f / (numBones * 4.0f);
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
         for(int i = 0; i < 4; i++){
            skinned += getMatrixFromTexture(s_texture3, vBones[i]) * getMatrixFromTexture(s_texture2, vBones[i]) * vertexPos * vWeights[i];
         }
         return skinned;
      }
	-->
	</source>
</shader>