<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision highp float;

		// From https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom

		// This shader performs downsampling on a texture,
		// as taken from Call Of Duty method, presented at ACM Siggraph 2014.
		// This particular method was customly designed to eliminate
		// "pulsating artifacts and temporal stability issues".

		// Remember to add bilinear minification filter for this texture!
		// Remember to use a floating-point texture format (for HDR)!
		// Remember to use edge clamping for this texture!
		uniform sampler2D s_texture0;
		uniform vec2 srcResolution;

		in vec2 v_texture;
		layout (location = 0) out vec3 downsample;

		uniform int prefilter;
		uniform float threshold;

		vec3 Prefilter (vec3 c) {
			float brightness = max(c.r, max(c.g, c.b));
			float contribution = max(0.0f, brightness - threshold);
			contribution /= max(brightness, 0.00001);
			return c * contribution;
		}

		void main()
		{
				vec2 srcTexelSize = 1.0 / srcResolution;
				float x = srcTexelSize.x;
				float y = srcTexelSize.y;

				// Take 13 samples around current texel:
				// a - b - c
				// - j - k -
				// d - e - f
				// - l - m -
				// g - h - i
				// === ('e' is the current texel) ===
				vec3 a = texture(s_texture0, vec2(v_texture.x - 2.0f*x, v_texture.y + 2.0f*y)).rgb;
				vec3 b = texture(s_texture0, vec2(v_texture.x,       v_texture.y + 2.0f*y)).rgb;
				vec3 c = texture(s_texture0, vec2(v_texture.x + 2.0f*x, v_texture.y + 2.0f*y)).rgb;

				vec3 d = texture(s_texture0, vec2(v_texture.x - 2.0f*x, v_texture.y)).rgb;
				vec3 e = texture(s_texture0, vec2(v_texture.x,       v_texture.y)).rgb;
				vec3 f = texture(s_texture0, vec2(v_texture.x + 2.0f*x, v_texture.y)).rgb;

				vec3 g = texture(s_texture0, vec2(v_texture.x - 2.0f*x, v_texture.y - 2.0f*y)).rgb;
				vec3 h = texture(s_texture0, vec2(v_texture.x,       v_texture.y - 2.0f*y)).rgb;
				vec3 i = texture(s_texture0, vec2(v_texture.x + 2.0f*x, v_texture.y - 2.0f*y)).rgb;

				vec3 j = texture(s_texture0, vec2(v_texture.x - x, v_texture.y + y)).rgb;
				vec3 k = texture(s_texture0, vec2(v_texture.x + x, v_texture.y + y)).rgb;
				vec3 l = texture(s_texture0, vec2(v_texture.x - x, v_texture.y - y)).rgb;
				vec3 m = texture(s_texture0, vec2(v_texture.x + x, v_texture.y - y)).rgb;

				// Apply weighted distribution:
				// 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
				// a,b,d,e * 0.125
				// b,c,e,f * 0.125
				// d,e,g,h * 0.125
				// e,f,h,i * 0.125
				// j,k,l,m * 0.5
				// This shows 5 square areas that are being sampled. But some of them overlap,
				// so to have an energy preserving downsample we need to make some adjustments.
				// The weights are the distributed, so that the sum of j,k,l,m (e.g.)
				// contribute 0.5 to the final color output. The code below is written
				// to effectively yield this sum. We get:
				// 0.125*5 + 0.03125*4 + 0.0625*4 = 1
				downsample = e*0.125;
				downsample += (a+c+g+i)*0.03125;
				downsample += (b+d+f+h)*0.0625;
				downsample += (j+k+l+m)*0.125;
				if(prefilter > 0){
					downsample = Prefilter(downsample);
				}
    		downsample = max(downsample, 0.0001f);
		}
	-->
	</source>
</shader>