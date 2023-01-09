<shader>
	<type name = "fragmentShader" />
	<include name = "camera.shader" />
	<source>
	<!--
		#version 300 es
		precision highp float;
		
		in vec2 v_texture;
		out vec4 fragColor;


		uniform sampler2D s_texture0; //Image to be processed 
		uniform sampler2D s_texture1; //Linear depth, where max value is far plane distance
		uniform vec2 uPixelSize; //The size of a pixel: vec2(1.0/width, 1.0/height) 
		uniform float focusPoint;
		uniform float focusScale;
		uniform float blurSize;
		uniform float radiusScale; // Smaller = nicer blur, larger = faster

		const float GOLDEN_ANGLE = 2.39996323; 

		float getBlurSize(float depth)
		{
			float coc = clamp((1.0 / max(focusPoint, 0.001) - 1.0 / max(depth, 0.001))*focusScale, -1.0, 1.0);
			return abs(coc) * blurSize;
		}

		vec3 depthOfField(vec2 texCoord)
		{
			vec3 color = texture(s_texture0, texCoord).rgb;
			if(focusScale == 0.0f){
				return color;
			}
			float centerDepth = -texture(s_texture1, texCoord).r;
			float centerSize = getBlurSize(centerDepth);
			float tot = 1.0;
			float radius = radiusScale;
			for (float ang = 0.0; radius<blurSize; ang += GOLDEN_ANGLE)
			{
				vec2 tc = texCoord + vec2(cos(ang), sin(ang)) * uPixelSize * radius;
				vec3 sampleColor = texture(s_texture0, tc).rgb;
				float sampleDepth = -texture(s_texture1, tc).r;
				float sampleSize = getBlurSize(sampleDepth);
				if (sampleDepth > centerDepth)
					sampleSize = clamp(sampleSize, 0.0, centerSize*2.0);
				float m = smoothstep(radius-0.5, radius+0.5, sampleSize);
				color += mix(color/tot, sampleColor, m);
				tot += 1.0;   radius += radiusScale/radius;
			}
			return color /= tot;
		}


		void main()
		{
			vec2 uv = vec2(v_texture.x, 1.0 - v_texture.y);
			fragColor = vec4(depthOfField(uv), 1.0f);
		}
	-->
	</source>
</shader>