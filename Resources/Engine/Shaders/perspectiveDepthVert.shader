<shader>
	<type name = "vertexShader" />
	<include name = "skinning.shader" />
	<include name = "camera.shader" />
	<uniform name = "ProjectViewModel" />
	<uniform name = "View" />
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

		out vec4 v_pos;
		out vec3 v_normal;
		out vec2 v_texture;
		out vec3 v_bitan;

		uniform mat4 ProjectViewModel;
		uniform mat4 View;
		uniform mat4 Model;
		uniform float Far;

		void main()
		{
			v_texture = vTexture;
			vec4 skinnedVPos = vec4(vPosition, 1.0);
			
			if(isSkinned > 0u){
				skin(skinnedVPos, skinnedVPos);
			}
			v_pos = (View * Model * skinnedVPos) / CamData.far;
		  gl_Position = ProjectViewModel * skinnedVPos;
			v_normal = vNormal;
			v_bitan = vBiTan;
		}
	-->
	</source>
</shader>