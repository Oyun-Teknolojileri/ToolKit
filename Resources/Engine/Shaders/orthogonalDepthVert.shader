<shader>
	<type name = "vertexShader" />
	<include name = "skinning.shader" />
	<uniform name = "ProjectViewModel" />
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

		uniform mat4 ProjectViewModel;
		uniform mat4 View;
    uniform mat4 Model;

		out vec2 v_texture;

		void main()
		{
			v_texture = vTexture;
			vec4 skinnedVPos = vec4(vPosition, 1.0);
			
			if(isSkinned > 0u){
				skinnedVPos = skin(skinnedVPos);
			}
		  gl_Position = ProjectViewModel * skinnedVPos;
		}
	-->
	</source>
</shader>