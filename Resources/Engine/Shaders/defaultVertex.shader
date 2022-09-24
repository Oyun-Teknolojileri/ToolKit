<shader>
	<type name = "vertexShader" />
	<uniform name = "ProjectViewModel" />
	<uniform name = "InverseTransModel" />
	<uniform name = "Model" />
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
      uniform mat4 Model;
      uniform sampler2D s_texture2;  // This is static data, bindPose texture
      uniform sampler2D s_texture3; // This is dynamic data, boneTransform texture
      uniform float numBones;
      uniform uint isSkinned;
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
            skinned += getMatrixFromTexture(s_texture3, vBones[i]) * getMatrixFromTexture(s_texture2, vBones[i]) * vertexPos * vWeights[i];
         }
         return skinned;
      }
      void main()
      {
         gl_Position = vec4(vPosition, 1.0f);
         if(isSkinned > 0u){
            gl_Position = skin(gl_Position);
         }
         
         v_pos = (Model * gl_Position).xyz;
         gl_Position = ProjectViewModel * gl_Position;
         v_texture = vTexture;
         v_normal = vNormal;
         v_bitan = vBiTan;
      };
	-->
	</source>
</shader>