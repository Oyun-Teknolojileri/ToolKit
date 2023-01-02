<shader>
	<type name = "fragmentShader" />
	<include name = "lighting.shader" />
	<include name = "ibl.shader" />
	<include name = "AO.shader" />
	<uniform name = "lightingType" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		// Position buffer
		uniform sampler2D s_texture9;
		// Normal buffer
		uniform sampler2D s_texture10;
		// Color buffer
		uniform sampler2D s_texture11;
		// Emissive buffer
		uniform sampler2D s_texture12;
		// Metallic roughness buffer
		uniform sampler2D s_texture14;

		uniform int aoEnabled;

		uniform vec3 camPos;

		/*
			lightingType:
			0 -> Phong
			1 -> PBR 
		*/
		uniform int lightingType;

		in vec2 v_texture;

		out vec4 fragColor;

		void main()
		{
			vec2 texCoord = vec2(v_texture.x, 1.0 - v_texture.y);
			vec3 position = texture(s_texture9, texCoord).rgb;
			vec3 normal = texture(s_texture10, texCoord).rgb;
			vec3 color = texture(s_texture11, texCoord).rgb;
			vec2 metallicRoughness = texture(s_texture14, texCoord).rg;
			vec3 emissive = texture(s_texture12, texCoord).rgb;

			vec3 n = normalize(normal);
			vec3 e = normalize(camPos - position);

			vec3 irradiance = vec3(0.0);
			if (lightingType == 0) // phong
			{
				irradiance = BlinnPhongLightingDeferred(position, n, e);
				irradiance *= color;
			}
			else // pbr
			{
				irradiance = PBRLightingDeferred(position, n, e, color, metallicRoughness.r, metallicRoughness.g);
			}

			irradiance += IblIrradiance(n);

			float ambientOcclusion;
			if (aoEnabled == 1)
			{
				ambientOcclusion = AmbientOcclusion();
			}
			else
			{
				ambientOcclusion = 1.0;
			}

			if (lightingType == 0)
			{
				irradiance += IblIrradiance(n);
			}
			else
			{
				irradiance += IBLIrradiancePBR(n, e, color.xyz, metallicRoughness.x, metallicRoughness.y);
			}

			fragColor = vec4(irradiance * ambientOcclusion, 1.0) + vec4(emissive, 0.0f);
		}
	-->
	</source>
</shader>