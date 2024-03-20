<shader>
	<type name = "fragmentShader" />
	<include name = "lighting.shader" />
	<include name = "ibl.shader" />
	<include name = "camera.shader" />
	<include name = "AO.shader" />
	<uniform name = "Color" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "useAlphaMask" />
	<uniform name = "alphaMaskTreshold" />
	<uniform name = "metallic" />
	<uniform name = "roughness" />
	<uniform name = "normalMapInUse" />
	<source>
	<!--
		#version 300 es
		precision highp float;
		precision highp float;
		precision highp sampler2D;
		precision highp samplerCube;
		precision highp sampler2DArray;
		precision highp int;

		uniform int useAlphaMask;
		uniform float alphaMaskTreshold;
		uniform vec4 Color;
		uniform int DiffuseTextureInUse;
		uniform sampler2D s_texture0;
		uniform sampler2D s_texture9; // normal
		uniform float metallic;
		uniform float roughness; 
		uniform int normalMapInUse;

		in vec3 v_pos;
		in vec3 v_normal;
		in vec2 v_texture;
		in mat3 TBN;

		out vec4 fragColor;

		void main()
		{
			vec4 color;
			if (DiffuseTextureInUse == 1)
			{
				color = texture(s_texture0, v_texture);
			}
			else
			{
				color = Color;
			}

#ifdef ENABLE_DISCARD_PIXEL
			if (useAlphaMask == 1)
			{
				if (color.a < alphaMaskTreshold)
				{
					discard;
				}
			}
#endif

			vec3 n;
			if (normalMapInUse == 1)
			{
				n = texture(s_texture9, v_texture).xyz;
				n = n * 2.0 - 1.0;
				n = TBN * n;
				n = normalize(n);
			}
			else
			{
				n = normalize(v_normal);
			}

			vec3 e = normalize(CamData.pos - v_pos);

			vec3 irradiance = PBRLighting(v_pos, n, e, CamData.pos, color.rgb, metallic, roughness);

			irradiance += IBLPhong(n);

			float ambientOcclusion = AmbientOcclusion();
			fragColor = vec4(irradiance * color.xyz * ambientOcclusion, color.a);
		}
	-->
	</source>
</shader>