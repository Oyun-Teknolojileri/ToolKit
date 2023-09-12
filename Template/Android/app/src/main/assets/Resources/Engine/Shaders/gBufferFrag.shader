<shader>
	<type name = "fragmentShader" />
	<include name = "camera.shader" />
	<include name = "ibl.shader" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "Color" />
	<uniform name = "useAlphaMask" />
	<uniform name = "alphaMaskTreshold" />
	<uniform name = "emissiveColor" />
	<uniform name = "emissiveTextureInUse" />
	<uniform name = "View" />
	<uniform name = "metallicRoughnessTextureInUse" />
	<uniform name = "metallic" />
	<uniform name = "roughness" />
  <uniform name = "normalMapInUse" />
	<uniform name = "lightingType" />
	<source>
	<!--
		#version 300 es
		precision highp float;
		precision highp int;

		in vec3 v_pos;
		in vec3 v_normal;
		in vec2 v_texture;
		in mat3 TBN;

		layout (location = 0) out vec3 fragPosition;
		layout (location = 1) out vec3 fragNormal;
		layout (location = 2) out vec3 fragColor;
		layout (location = 3) out vec3 fragEmissive;
		layout (location = 4) out vec3 fragLinearDepth;
		layout (location = 5) out vec2 fragMetallicRoughess;
		layout (location = 6) out vec3 fragIbl;

		uniform int DiffuseTextureInUse;
		uniform sampler2D s_texture0; // color
		uniform sampler2D s_texture1; // emissive
		uniform sampler2D s_texture4; // metallic-roughness
		uniform sampler2D s_texture9; // normal
		uniform vec4 Color;
		uniform vec3 emissiveColor;

		uniform int useAlphaMask;
		uniform float alphaMaskTreshold;
		uniform int emissiveTextureInUse;

		uniform int metallicRoughnessTextureInUse;
		uniform float metallic;
		uniform float roughness;

		uniform int normalMapInUse;

		uniform mat4 View;

		uniform int lightingType;

		void main()
		{
			vec4 color;
			if (DiffuseTextureInUse == 1)
			{
				color = texture(s_texture0, v_texture).rgba;
			}
			else
			{
				color = Color;
			}

			if (useAlphaMask == 1)
			{
				if (color.a < alphaMaskTreshold)
				{
					discard;
				}
			}

			if(emissiveTextureInUse == 1){
				fragEmissive = texture(s_texture1, v_texture).rgb;
			}
			else{
				fragEmissive = emissiveColor;
			}

		  fragPosition = v_pos;

			if (normalMapInUse == 1)
			{
				fragNormal = texture(s_texture9, v_texture).xyz;
				fragNormal = fragNormal * 2.0 - 1.0;
				fragNormal = TBN * fragNormal;
				fragNormal = normalize(fragNormal);
			}
			else
			{
		  	fragNormal = normalize(v_normal);
			}

			fragColor = color.xyz;
			vec4 viewPnt = View * vec4(v_pos, 1.0);
			fragLinearDepth = viewPnt.xyz;

			if (metallicRoughnessTextureInUse == 1)
			{
				fragMetallicRoughess = texture(s_texture4, v_texture).rg;
			}
			else
			{
				fragMetallicRoughess = vec2(metallic, roughness);
			}

			vec3 fragToEye = normalize(CamData.pos - v_pos);
			fragIbl = IBLPBR(fragNormal, fragToEye, fragColor.xyz, fragMetallicRoughess.x, fragMetallicRoughess.y);
		}
	-->
	</source>
</shader>
