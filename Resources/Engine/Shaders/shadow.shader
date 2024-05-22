<shader>
	<type name = "includeShader" />
	<include name = "VSM.shader" />
	<source>
	<!--
		float PCFFilterShadow2D
    (
      sampler2DArray shadowAtlas,
      vec3 uvLayer,
      vec2 coordStart,
      vec2 coordEnd,
      int samples,
      float radius,
      float currDepth,
      float LBR,
      float shadowBias
    )
		{
      // Single pass average filter the shadow map.
			vec2 sum = vec2(0.0);
			for (int i = 0; i < samples; ++i)
			{
        // Below is randomized sample which causes noise rather than banding.
        // Use di for sampling the disk to see the effect.
        // int di = int(127.0 * Random( vec4( gl_FragCoord.xyy, float(i) ) )) % 127;
				vec2 offset = PoissonDisk[i].xy * radius;

        vec3 texCoord = uvLayer;
        texCoord.xy = ClampTextureCoordinates(uvLayer.xy + offset, coordStart, coordEnd);
				sum += texture(shadowAtlas, texCoord).xy;
			}

      // Filtered depth & depth^2
      sum = sum / float(samples);

      vec2 exponents = EvsmExponents;
      vec2 warpedDepth = WarpDepth(currDepth, exponents);

      // Derivative of warping at depth
      vec2 depthScale = 100.0 * shadowBias * exponents * warpedDepth;
      vec2 minVariance = depthScale * depthScale;

      return ChebyshevUpperBound(sum, warpedDepth.x, minVariance.x, LBR);
		}

    float PCFFilterOmni
    (
      sampler2DArray shadowAtlas,
      vec2 startCoord,
      float shadowAtlasResRatio,
      float shadowAtlasLayer,
      vec3 dir,
      int samples,
      float radius,
      float currDepth,
      float LBR,
      float shadowBias
    )
    {
      // Single pass average filter the shadow map.
			vec2 sum = vec2(0.0);
			for (int i = 0; i < samples; ++i)
			{
        // Below is randomized sample which causes noise rather than banding.
        // Use di for sampling the disk to see the effect.
        // int di = int(127.0 * Random( vec4( gl_FragCoord.xyy, float(i) ) )) % 127;
				vec3 offset = PoissonDisk[i] * radius;

        // Cubemap sample
        vec3 sampleDir = dir + offset;
        vec3 texCoord = UVWToUVLayer(sampleDir);
        texCoord.xy = startCoord + shadowAtlasResRatio * texCoord.xy;
        texCoord.z = shadowAtlasLayer + texCoord.z;
				sum += texture(shadowAtlas, texCoord).xy;
			}

      // Filtered depth & depth^2
      sum = sum / float(samples);

      vec2 exponents = EvsmExponents;
      vec2 warpedDepth = WarpDepth(currDepth, exponents);

      // Derivative of warping at depth
      vec2 depthScale = 100.0 * shadowBias * exponents * warpedDepth;
      vec2 minVariance = depthScale * depthScale;

      return ChebyshevUpperBound(sum, warpedDepth.x, minVariance.x, LBR);
    }
	-->
	</source>
</shader>