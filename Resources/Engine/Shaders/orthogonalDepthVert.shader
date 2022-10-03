<shader>
	<type name = "vertexShader" />
	<uniform name = "ProjectViewModel" />
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

		uniform mat4 ProjectViewModel;

		out vec3 v_pos;
		out vec3 v_normal;
		out vec2 v_texture;
		out vec3 v_bitan;
		
		void main()
		{
			v_texture = vTexture;
			vec4 skinnedVPos = vec4(vPosition, 1.0);
			
			if(isSkinned > 0u){
				skinnedVPos = skin(skinnedVPos);
			}
		  gl_Position = ProjectViewModel * skinnedVPos;
			v_pos = skinnedVPos;
			v_normal = vNormal;
			v_bitan = vBiTan;
		}
	-->
	</source>
</shader>