<shader>
	<type name = "fragmentShader" />
	<include name = "pbr.shader" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		uniform samplerCube s_texture6;

		uniform float resPerFace;
		uniform float roughness;

		in vec3 v_pos;
		out vec4 fragColor;

		const float FLT_MAX = 3.402823466e+38;

		// https://learnopengl.com/PBR/IBL/Specular-IBL
		void main()
		{		
			const uint SAMPLE_COUNT = 1024u;

			vec3 N = normalize(v_pos);
			
			// Make the simplifying assumption that V equals R equals the normal 
			vec3 R = N;
			vec3 V = R;

			vec3 prefilteredColor = vec3(0.0);
			float totalWeight = 0.0;

			for(uint i = 0u; i < SAMPLE_COUNT; ++i)
			{
				// Generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
				vec2 Xi = Hammersley(i, SAMPLE_COUNT);
				vec3 H = ImportanceSampleGGX(Xi, N, roughness);
				vec3 L  = normalize(2.0 * dot(V, H) * H - V);

				float NdotL = max(dot(N, L), 0.0);
				if(NdotL > 0.0)
				{
					// Sample from the environment's mip level based on roughness/pdf
					float D   = DistributionGGX(N, H, roughness);
					float NdotH = max(dot(N, H), 0.0);
					float HdotV = max(dot(H, V), 0.0);
					float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 

					float saTexel  = 4.0 * PI / (6.0 * resPerFace * resPerFace);
					float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

					float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 
					
					vec3 texel = textureLod(s_texture6, L, mipLevel).rgb;
					texel  = clamp(texel, vec3(0.0), vec3(FLT_MAX));
					prefilteredColor +=  texel * NdotL;
					
					totalWeight      += NdotL;
				}
			}

			prefilteredColor = prefilteredColor / totalWeight;

			fragColor = vec4(prefilteredColor, 1.0);
		} 
	-->
	</source>
</shader>