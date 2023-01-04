<shader>
	<type name = "fragmentShader" />
	<include name = "pbr.shader" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		in vec2 v_texture;
		out vec4 fragColor;

		// https://learnopengl.com/PBR/IBL/Specular-IBL
		vec2 IntegrateBRDF(float NdotV, float roughness)
		{
			const uint SAMPLE_COUNT = 1024u;

			vec3 V;
			V.x = sqrt(1.0 - NdotV*NdotV);
			V.y = 0.0;
			V.z = NdotV;

			float A = 0.0;
			float B = 0.0;

			vec3 N = vec3(0.0, 0.0, 1.0);

			for(uint i = 0u; i < SAMPLE_COUNT; ++i)
			{
				vec2 Xi = Hammersley(i, SAMPLE_COUNT);
				vec3 H  = ImportanceSampleGGX(Xi, N, roughness);
				vec3 L  = normalize(2.0 * dot(V, H) * H - V);

				float NdotL = max(L.z, 0.0);
				float NdotH = max(H.z, 0.0);
				float VdotH = max(dot(V, H), 0.0);

				if(NdotL > 0.0)
				{
					float G = GeometrySmithForIBL(N, V, L, roughness);
					float G_Vis = (G * VdotH) / (NdotH * NdotV);
					float Fc = pow(1.0 - VdotH, 5.0);

					A += (1.0 - Fc) * G_Vis;
					B += Fc * G_Vis;
				}
			}

			A /= float(SAMPLE_COUNT);
			B /= float(SAMPLE_COUNT);
			return vec2(A, B);
		}

		void main() 
		{
			vec2 integratedBRDF = IntegrateBRDF(v_texture.x, 1.0 - v_texture.y);
			fragColor = vec4(integratedBRDF, 0.0, 0.0);
		}
	-->
	</source>
</shader>