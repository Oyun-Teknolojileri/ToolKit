<shader>
	<type name = "vertexShader" />
	<include name = "skinning.shader" />
	<uniform name = "ProjectViewModel" />
	<uniform name = "InverseTransModel" />
	<uniform name = "Model" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		// Fixed Attributes.
		layout (location = 0) in vec3 vPosition;
		layout (location = 1) in vec3 vNormal;
		layout (location = 2) in vec2 vTexture;
		layout (location = 3) in vec3 vBiTan;

		// Declare used variables in the xml unifrom attribute.
		uniform mat4 ProjectViewModel;
		uniform mat4 InverseTransModel;
		uniform mat4 Model;
		uniform mat4 viewMatrix;
		uniform uint isSkinned;

		out vec3 v_pos;
		out vec3 v_normal;
		out vec2 v_texture;
		out vec3 v_bitan;

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

			//v_normal = vNormal;
			v_bitan = vBiTan;
		}
	-->
	</source>
</shader>