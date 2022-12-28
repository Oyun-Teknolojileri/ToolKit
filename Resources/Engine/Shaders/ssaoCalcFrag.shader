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
		uniform sampler2D s_texture1; // normal
		uniform sampler2D s_texture2; // noise
		uniform sampler2D s_texture3; // linear depth

		uniform vec2 screen_size;
		uniform mat4 viewMatrix;

		uniform vec3 samples[64];

		uniform mat4 projection;

		// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
		int kernelSize = 64;
		float radius = 0.5;
		float bias = 0.025;

		// tile noise texture over screen based on screen dimensions divided by noise size

		void main()
		{
			vec2 texCoord = vec2(v_texture.x, 1.0 - v_texture.y);
			vec3 fragPos = vec3(viewMatrix * vec4(texture(s_texture0, texCoord).xyz, 1.0));

		  vec2 noiseScale = vec2(screen_size.x / 4.0, screen_size.y / 4.0); 
			vec3 normal = texture(s_texture1, texCoord).rgb;
			vec3 randomVec = vec3(texture(s_texture2, texCoord * noiseScale).xy, 0.0);
			// create TBN change-of-basis matrix: from tangent-space to view-space
			vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
			vec3 bitangent = cross(normal, tangent);
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
				float sampleDepth = texture(s_texture3, offset.xy).r; // get depth value of kernel sample
				
				// range check & accumulate
				float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
				occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;           
			}
			occlusion = 1.0 - (occlusion / float(kernelSize));
			
			fragColor = vec4(occlusion, 0.0, 0.0, 1.0);
		}

	-->
	</source>
</shader>