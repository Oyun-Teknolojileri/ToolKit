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
      uniform Bone bones[64];
      out vec3 v_pos;
      out vec3 v_normal;
      out vec2 v_texture;
      out vec3 v_bitan;
      void main()
      {
         gl_Position = bones[vBones.x].transform * bones[vBones.x].bindPose * vec4(vPosition, 1.0) * vWeights.x;
         gl_Position += bones[vBones.y].transform * bones[vBones.y].bindPose * vec4(vPosition, 1.0) * vWeights.y;
         gl_Position += bones[vBones.z].transform * bones[vBones.z].bindPose * vec4(vPosition, 1.0) * vWeights.z;
         gl_Position += bones[vBones.w].transform * bones[vBones.w].bindPose * vec4(vPosition, 1.0) * vWeights.w;
         v_pos = gl_Position.xyz;
         gl_Position = ProjectViewModel * gl_Position;
         v_texture = vTexture;
         v_normal = vNormal;
         v_bitan = vBiTan;
      };
	-->
	</source>
</shader>