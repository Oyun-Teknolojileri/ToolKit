<shader>
	<type name = "fragmentShader" />
	<uniform name = "emissiveColorMultiplier" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "Color" />
	<source>
	<!--
		#version 300 es
		precision mediump float;

		uniform sampler2D s_texture0;
		uniform int DiffuseTextureInUse;
		uniform vec4 Color;
		uniform vec3 emissiveColorMultiplier;

		in vec2 v_texture;
		out vec4 fragColor;

		void main()
		{
			vec4 frag;
			if(DiffuseTextureInUse > 0){
		  	frag = texture(s_texture0, v_texture);
			}
			else{
				frag = Color;
			}
			fragColor = frag * vec4(emissiveColorMultiplier, 1.0f);
		}
	-->
	</source>
</shader>