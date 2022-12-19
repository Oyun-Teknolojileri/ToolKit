<shader>
	<type name = "fragmentShader" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "Color" />
	<uniform name = "emissiveColor" />
	<uniform name = "emissiveTextureInUse" />
	<include name = "lighting.shader" />
	<include name = "ibl.shader" />
	<include name = "AO.shader" />
	<include name = "camera.shader" />
	<source>
	<!--
		#version 300 es
		precision mediump float;

		uniform sampler2D s_texture0;
		uniform sampler2D s_texture1;
		uniform int DiffuseTextureInUse;
		uniform int emissiveTextureInUse;
		uniform vec4 Color;
		uniform vec3 emissiveColor;


		in vec3 v_pos;
		in vec3 v_normal;
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
			vec3 emissive;
			if(emissiveTextureInUse > 0){
				emissive = texture(s_texture1, v_texture).xyz;
			}
			else{
				emissive = emissiveColor;
			}

			vec3 n = normalize(v_normal);
			vec3 e = normalize(CamData.pos - v_pos);

			vec3 irradiance = BlinnPhongLighting(v_pos, n, e);

			irradiance += IblIrradiance(n);

			// float ambientOcclusion = AmbientOcclusion();

			fragColor = (vec4(irradiance, 1.0) * frag) + vec4(emissive, 0.0f);
		}
	-->
	</source>
</shader>