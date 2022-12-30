<shader>
	<type name = "fragmentShader" />
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
	<source>
	<!--
		#version 300 es
		precision highp float;

		in vec3 v_pos;
		in vec3 v_normal;
		in vec2 v_texture;

		layout (location = 0) out vec3 fragPosition;
		layout (location = 1) out vec3 fragNormal;
		layout (location = 2) out vec3 fragColor;
		layout (location = 3) out vec3 fragEmissive;
		layout (location = 4) out float fragLinearDepth;
		layout (location = 5) out vec2 fragMetallicRoughess;

		uniform int DiffuseTextureInUse;
		uniform sampler2D s_texture0; // color
		uniform sampler2D s_texture1; // emissive
		uniform sampler2D s_texture4; // metallic-roughness
		uniform vec4 Color;
		uniform vec3 emissiveColor;

		uniform int useAlphaMask;
		uniform float alphaMaskTreshold;
		uniform int emissiveTextureInUse;

		uniform int metallicRoughnessTextureInUse;
		uniform float metallic;
		uniform float roughness;

		uniform mat4 View;

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
		  fragNormal = normalize(v_normal);
			fragColor = color.xyz;
			fragLinearDepth = (View * vec4(v_pos, 1.0)).z;

			vec2 metallicRoughness;
			if (metallicRoughnessTextureInUse == 1)
			{
				metallicRoughness = texture(s_texture4, v_texture).rg;
			}
			else
			{
				metallicRoughness = vec2(metallic, roughness);
			}
			fragMetallicRoughess = metallicRoughness;
		}
	-->
	</source>
</shader>
