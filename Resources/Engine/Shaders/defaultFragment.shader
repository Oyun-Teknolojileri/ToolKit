<shader>
	<type name = "fragmentShader" />
	<include name = "lighting.shader" />
	<include name = "ibl.shader" />
	<include name = "camera.shader" />
	<include name = "AO.shader" />
	<uniform name = "CamData" />
	<uniform name = "useAlphaMask" />
	<uniform name = "alphaMaskTreshold" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "Color" />
	<uniform name = "emissiveColor" />
	<uniform name = "emissiveTextureInUse" />
	<uniform name = "lightingType" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		uniform sampler2D s_texture0;
		uniform sampler2D s_texture1;
		uniform int LightingOnly;
		uniform int useAlphaMask;
		uniform float alphaMaskTreshold;
		uniform int DiffuseTextureInUse;
		uniform vec4 Color;
		uniform int emissiveTextureInUse;
		uniform vec3 emissiveColor;

		/*
			lightingType:
			0 -> Phong
			1 -> PBR 
		*/
		uniform int lightingType;

		in vec3 v_pos;
		in vec3 v_normal;
		in vec2 v_texture;

		out vec4 fragColor;

		void main()
		{
			vec4 color;
			if(DiffuseTextureInUse > 0)
			{
		  	color = texture(s_texture0, v_texture);
			}
			else
			{
				color = Color;
			}
			vec3 emissive;
			if(emissiveTextureInUse > 0){
				emissive = texture(s_texture1, v_texture).xyz;
			}
			else{
				emissive = emissiveColor;
			}

			if (useAlphaMask == 1)
			{
				if(color.a < alphaMaskTreshold){
					discard;
				}
			}
			if (LightingOnly == 1)
			{
				color.xyz = vec3(1.0);
			}

			vec3 n = normalize(v_normal);
			vec3 e = normalize(CamData.pos - v_pos);

			vec3 irradiance = vec3(0.0);
			if (lightingType == 0)
			{
				irradiance = BlinnPhongLighting(v_pos, n, e);
				irradiance *= color.xyz;
			}
			else
			{
				irradiance = PBRLighting(v_pos, n, e, color.xyz);
			}

			irradiance += IblIrradiance(n);

			// float ambientOcclusion = AmbientOcclusion();

			fragColor = vec4(irradiance, color.a) + vec4(emissive, 0.0f);
		}
	-->
	</source>
</shader>