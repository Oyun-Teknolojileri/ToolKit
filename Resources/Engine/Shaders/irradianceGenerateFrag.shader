<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		uniform samplerCube s_texture1;

		in vec3 v_pos;
		out vec4 fragColor;

		const float PI = 3.14159265359;

		// reference: https://learnopengl.com/PBR/IBL/Diffuse-irradiance
		void main()
		{
	    vec3 N = normalize(v_pos);

	    vec3 irradiance = vec3(0.0); 
	    
	    // tangent space calculation from origin point
	    vec3 up = vec3(0.0, 1.0, 0.0);
	    vec3 right = normalize(cross(up, N));
	    up = normalize(cross(N, right));

	    float sampleDelta = 0.025;
	    float nrSamples = 0.0;
	    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	    {
	        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
	        {
	            // spherical to cartesian (in tangent space)
	            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
	            // tangent space to world
	            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

	            irradiance += texture(s_texture1, sampleVec).rgb * cos(theta) * sin(theta);
	            nrSamples += 1.0;
	        }
	    }
	    irradiance = (PI * irradiance) / nrSamples;

	    fragColor = vec4(irradiance, 1.0);
		}
	-->
	</source>
</shader>