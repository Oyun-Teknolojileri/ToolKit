<shader>
	<type name = "includeShader" />
	<include name = "VSM.shader" />
	<source>
	<!--

  #ifndef SHADOW_SHADER
  #define SHADOW_SHADER

  #define MAX_CASCADE_COUNT 4
  #define SHADOW_ATLAS_SIZE 2048.0

  /*
  * Given a shadow map size and start coordinates, finds the queried shadow map's layer and start coordinates.
  * Shadow maps for cascades and cube maps are placed sequentially to the atlas. So if you pass the directional
  * light's start location and start layer and query the layer and start coordinate of 4'th cascade, this function
  * will calculate it.
  */
  void ShadowAtlasLut(in float size, in vec2 startCoord, in int queriedMap, out int layer, out vec2 targetCoord)
  {
    targetCoord = startCoord;
    layer = 0;
	
    for (int i = 1; i <= queriedMap; i++)
    {
	    targetCoord.x += size;

      if (targetCoord.x >= SHADOW_ATLAS_SIZE)
      {
        targetCoord.x = 0.0;
        targetCoord.y += size;
        if (targetCoord.y >= SHADOW_ATLAS_SIZE)
        {
          layer += 1;
          targetCoord.x = 0.0;
          targetCoord.y = 0.0;
        }
      }
    }
  }

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
	vec4 occuluderAverage = vec4(0.0);
	for (int i = 0; i < samples; ++i)
	{
    // Below is randomized sample which causes noise rather than banding.
    // Use di for sampling the disk to see the effect.
    // int di = int(127.0 * Random( vec4( gl_FragCoord.xyy, float(i) ) )) % 127;
		vec2 offset = PoissonDisk[i].xy * radius;

    vec3 texCoord = uvLayer;
    texCoord.xy = ClampTextureCoordinates(uvLayer.xy + offset, coordStart, coordEnd);
		occuluderAverage += texture(shadowAtlas, texCoord);
	}

  // Filtered depth & depth^2
	occuluderAverage = occuluderAverage / float(samples);

  vec2 exponents = EvsmExponents;
  vec2 warpedDepth = WarpDepth(currDepth, exponents);

  // Derivative of warping at depth
  vec2 depthScale = 100.0 * shadowBias * exponents * warpedDepth;
  vec2 minVariance = depthScale * depthScale;

	float posContrib = ChebyshevUpperBound(occuluderAverage.xz, warpedDepth.x, minVariance.x, LBR);
	float negContrib = ChebyshevUpperBound(occuluderAverage.yw, warpedDepth.y, minVariance.y, LBR);
	return min(posContrib, negContrib);
}

float PCFFilterOmni
(
  sampler2DArray shadowAtlas,
  vec2 startCoord,
  float shadowAtlasResRatio,
  int shadowAtlasLayer,
  vec3 dir,
  int samples,
  float radius,
  float currDepth,
  float LBR,
  float shadowBias
)
{
  vec2 halfPixel = vec2((1.0 / SHADOW_ATLAS_SIZE) * 0.5);

  // Single pass average filter the shadow map.
	vec4 occuluderAverage = vec4(0.0);
	for (int i = 0; i < samples; ++i)
	{
    // Below is randomized sample which causes noise rather than banding.
    // Use di for sampling the disk to see the effect.
    // int di = int(127.0 * Random( vec4( gl_FragCoord.xyy, float(i) ) )) % 127;
		vec3 offset = PoissonDisk[i] * radius;

    // Adhoc coefficient 50.0
    // If offset applied to texCoord, wrong face may be sampled due to bleeding.

    // Cubemap sample
    vec3 texCoord = UVWToUVLayer(dir + offset * 50.0);

    int face = int(texCoord.z);

    int layer = 0;
		vec2 coord = vec2(0.0);
		float shadowMapSize = shadowAtlasResRatio * SHADOW_ATLAS_SIZE;
		ShadowAtlasLut(shadowMapSize, startCoord, face, layer, coord);
    coord /= SHADOW_ATLAS_SIZE;

		layer += shadowAtlasLayer;

    vec2 beginCoord = coord;
    vec2 endCoord = beginCoord + shadowAtlasResRatio;

    texCoord.xy = beginCoord + (shadowAtlasResRatio * texCoord.xy);
    texCoord.z = float(layer);

    // Keep the pixel always in the corresponding face, prevent bleeding.
    texCoord.xy = clamp(texCoord.xy, beginCoord + halfPixel, endCoord - halfPixel);

		occuluderAverage += texture(shadowAtlas, texCoord);
	}

  // Filtered depth & depth^2
	occuluderAverage = occuluderAverage / float(samples);

  vec2 exponents = EvsmExponents;
  vec2 warpedDepth = WarpDepth(currDepth, exponents);

  // Derivative of warping at depth
  vec2 depthScale = 100.0 * shadowBias * exponents * warpedDepth;
  vec2 minVariance = depthScale * depthScale;

	float posContrib = ChebyshevUpperBound(occuluderAverage.xz, warpedDepth.x, minVariance.x, LBR);
	float negContrib = ChebyshevUpperBound(occuluderAverage.yw, warpedDepth.y, minVariance.y, LBR);
	return min(posContrib, negContrib);
}

#endif

	-->
	</source>
</shader>