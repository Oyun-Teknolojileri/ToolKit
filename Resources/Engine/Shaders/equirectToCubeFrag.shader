<shader>
	<type name = "fragmentShader" />
	<uniform name = "Exposure" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		uniform float Exposure;
		uniform sampler2D equirectangularMap;

		in vec3 v_pos;
		out vec4 fragColor;

		const vec2 invAtan = vec2(0.1591, 0.3183);
		vec2 SampleSphericalMap(vec3 v)
		{
		    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
		    uv *= invAtan;
		    uv += 0.5;
		    return uv;
		}

		void main()
		{
	    vec2 uv = SampleSphericalMap(normalize(v_pos));
	    vec3 color = texture(equirectangularMap, uv).rgb;

			// Tone mapping
			color = vec3(1.0) - exp(-color * Exposure);
			// Gamma correct
			color = pow(color, vec3(1.0 / 2.2));

	    fragColor = vec4(color, 1.0);
		}
	-->
	</source>
</shader>