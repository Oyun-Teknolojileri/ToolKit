<shader>
	<type name = "fragmentShader" />
	<uniform name = "Color" />
	<source>
	<!--
		#version 300 es
		precision highp float;
		
		uniform vec4 Color;
		out vec4 fragColor;

		in vec3 v_pos;
		in vec3 v_normal;
		in vec2 v_texture;

		uniform sampler2D s_texture0; // position (in world space)
		uniform sampler2D s_texture1; // normal (in world space)
		uniform sampler2D s_texture2; // noise
		uniform sampler2D s_texture3; // linear depth

		uniform vec2 screenSize;
		uniform mat4 viewMatrix;
		uniform vec3 samples[64];
		uniform mat4 projection;
		uniform float radius;
		uniform float bias;
		
		const int kernelSize = 64;

		void main()
		{
			vec2 texCoord = vec2(v_texture.x, 1.0 - v_texture.y);
			vec3 fragPos = texture(s_texture3, texCoord).xyz; // View pos.

			// tile noise texture over screen based on screen dimensions divided by noise size
			vec2 noiseScale = vec2(screenSize.x / 4.0, screenSize.y / 4.0); 
			vec3 normal = texture(s_texture1, texCoord).rgb;
			mat3 invTrsView = (transpose(inverse(mat3(viewMatrix))));
			normal = normalize(invTrsView * normal); // World to View
			vec3 randomVec = vec3(texture(s_texture2, texCoord * noiseScale).xy, 0.0);
			
			// create TBN change-of-basis matrix: from tangent-space to view-space
			vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
			vec3 bitangent = normalize(cross(normal, tangent));
			mat3 TBN = mat3(tangent, bitangent, normal);
			
			// iterate over the sample kernel and calculate occlusion factor
			float occlusion = 0.0;
			for(int i = 0; i < kernelSize; ++i)
			{
				// get sample position
				vec3 samplePos = TBN * samples[i]; // from tangent to view-space
				samplePos = fragPos + samplePos * radius; 
				
				// project sample position (to sample texture) (to get position on screen/texture)
				vec4 offset = vec4(samplePos, 1.0);
				offset = projection * offset; // from view to clip-space
				offset.xyz /= offset.w; // perspective divide
				offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
				
				// get sample depth
				float sampleDepth = texture(s_texture3, offset.xy).z; // get depth value of kernel sample
				
				// range check & accumulate
				float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
				occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;           
			}
			occlusion = max(1.0 - (occlusion / float(kernelSize)), 0.0);
			fragColor = vec4(occlusion, 0.0, 0.0, 1.0);
		}

	-->
	</source>
</shader>