<shader>
	<type name = "fragmentShader" />
	<include name = "lighting.shader" />
	<include name = "ibl.shader" />
	<include name = "camera.shader" />
	<include name = "AO.shader" />
	<uniform name = "CamData" />
	<source>
	<!--
	// Don't forget to apply changes to lightingOnly shader
		#version 300 es
		precision highp float;

		uniform sampler2D s_texture0;

		in vec3 v_pos;
		in vec3 v_normal;
		in vec2 v_texture;

		out vec4 fragColor;

		void main()
		{
			vec4 objectColor = texture(s_texture0, v_texture);
			if(objectColor.a < 0.1f){
				discard;
			}

			vec3 n = normalize(v_normal);
			vec3 e = normalize(CamData.pos - v_pos);

			vec3 irradiance = BlinnPhongLighting(v_pos, n, e);

			irradiance += IblIrradiance(n);

			float ambientOcclusion = AmbientOcclusion();

			fragColor = vec4(ambientOcclusion * irradiance, 1.0);
		}
	-->
	</source>
</shader>