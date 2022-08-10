<shader>
	<type name = "vertexShader" />
	<uniform name = "ProjectViewModel" />
	<source>
	<!--
      #version 300 es
      struct Bone
      {
         mat4 transform;
         mat4 bindPose;
      };
      in vec3 vPosition;
      in vec3 vNormal;
      in vec2 vTexture;
      in vec3 vBiTan;
      in uvec4 vBones;
      in vec4 vWeights;
      uniform mat4 ProjectViewModel;
      uniform sampler2D boneTransform_textures; // This is dynamic data
      uniform sampler2D boneBindPose_textures;  // This is static data
      uniform float numBones;
      out vec3 v_pos;
      out vec3 v_normal;
      out vec2 v_texture;
      out vec3 v_bitan;
      
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
            skinned += getMatrixFromTexture(boneTransform_textures, vBones[i]) * getMatrixFromTexture(boneBindPose_textures, vBones[i]) * vertexPos * vWeights[i];
         }
         return skinned;
      }
      void main()
      {
         gl_Position = skin(vec4(vPosition, 1.0f));
         v_pos = gl_Position.xyz;
         gl_Position = ProjectViewModel * gl_Position;
         v_texture = vTexture;
         v_normal = vNormal;
         v_bitan = vBiTan;
      };
	-->
	</source>
</shader>