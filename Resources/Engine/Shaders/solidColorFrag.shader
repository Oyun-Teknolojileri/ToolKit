<shader>
	<type name = "fragmentShader" />
	<include name = "lighting.shader" />
	<include name = "ibl.shader" />
	<include name = "camera.shader" />
	<uniform name = "CamData" />
	<uniform name = "Color" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "useAlphaMask" />
	<uniform name = "alphaMaskTreshold" />
	<uniform name = "metallic" />
	<uniform name = "roughness" />
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
		uniform float metallic;
    uniform float roughness; 

		in vec3 v_pos;
		in vec3 v_normal;
		in vec2 v_texture;

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

			if (useAlphaMask == 1)
			{
				if (color.a < alphaMaskTreshold)
				{
					discard;
				}
			}

			vec3 n = normalize(v_normal);
			vec3 e = normalize(CamData.pos - v_pos);

			vec3 irradiance = PBRLighting(v_pos, n, e, color.rgb, metallic, roughness);

			irradiance += IBLPhong(n);

			fragColor = vec4(irradiance * color.xyz, color.a);
		}
	-->
	</source>
</shader>