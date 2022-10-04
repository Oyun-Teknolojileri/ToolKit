<shader>
	<type name = "vertexShader" />
	<uniform name = "ProjectViewModel" />
	<uniform name = "InverseTransModel" />
	<uniform name = "Model" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		// Fixed Attributes.
		in vec3 vPosition;
		in vec3 vNormal;
		in vec2 vTexture;
		in vec3 vBiTan;
		in uvec4 vBones;
		in vec4 vWeights;

		// Declare used variables in the xml unifrom attribute.
		uniform mat4 ProjectViewModel;
		uniform mat4 InverseTransModel;
		uniform mat4 Model;
		uniform mat4 viewMatrix;

		out vec3 v_pos;
		out vec3 v_normal;
		out vec2 v_texture;
		out vec3 v_bitan;
		
		uniform sampler2D s_texture2;  // This is static data, bindPose texture
		uniform sampler2D s_texture3; // This is dynamic data, boneTransform texture
		uniform float numBones;
		uniform uint isSkinned;
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
			vec4 localv_pos = vec4(vPosition, 1.0f);
			if(isSkinned > 0u){
				localv_pos = skin(localv_pos);
			}
		  v_pos = (viewMatrix * Model * localv_pos).xyz;
		  gl_Position = ProjectViewModel * localv_pos;
		  mat3 normalMat = transpose(inverse(mat3(viewMatrix * Model)));
		  v_normal = normalMat * vNormal;
		  v_texture = vTexture;

			v_normal = vNormal;
			v_bitan = vBiTan;
		}
	-->
	</source>
</shader>