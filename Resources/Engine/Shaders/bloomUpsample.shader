<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision mediump float;
		precision lowp sampler2D;
		precision lowp int;

		// From https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom

		// This shader performs upsampling on a texture,
		// as taken from Call Of Duty method, presented at ACM Siggraph 2014.

		// Remember to add bilinear minification filter for this texture!
		// Remember to use a floating-point texture format (for HDR)!
		// Remember to use edge clamping for this texture!
		uniform sampler2D s_texture0;
		uniform float filterRadius;
		uniform float intensity;

		in vec2 v_texture;
		layout (location = 0) out vec3 upsample;

		void main()
		{
				// The filter kernel is applied with a radius, specified in texture
				// coordinates, so that the radius will vary across mip resolutions.
				float x = filterRadius;
				float y = filterRadius;

				// Take 9 samples around current texel:
				// a - b - c
				// d - e - f
				// g - h - i
				// === ('e' is the current texel) ===
				vec3 a = texture(s_texture0, vec2(v_texture.x - x, v_texture.y + y)).rgb;
				vec3 b = texture(s_texture0, vec2(v_texture.x,     v_texture.y + y)).rgb;
				vec3 c = texture(s_texture0, vec2(v_texture.x + x, v_texture.y + y)).rgb;

				vec3 d = texture(s_texture0, vec2(v_texture.x - x, v_texture.y)).rgb;
				vec3 e = texture(s_texture0, vec2(v_texture.x,     v_texture.y)).rgb;
				vec3 f = texture(s_texture0, vec2(v_texture.x + x, v_texture.y)).rgb;

				vec3 g = texture(s_texture0, vec2(v_texture.x - x, v_texture.y - y)).rgb;
				vec3 h = texture(s_texture0, vec2(v_texture.x,     v_texture.y - y)).rgb;
				vec3 i = texture(s_texture0, vec2(v_texture.x + x, v_texture.y - y)).rgb;

				// Apply weighted distribution, by using a 3x3 tent filter:
				//  1   | 1 2 1 |
				// -- * | 2 4 2 |
				// 16   | 1 2 1 |
				upsample = e*4.0;
				upsample += (b+d+f+h)*2.0;
				upsample += (a+c+g+i);
				upsample *= 1.0 / 16.0;
				upsample *= intensity;
		}
	-->
	</source>
</shader>