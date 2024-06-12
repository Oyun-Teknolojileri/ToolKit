<shader>
	<type name = "includeShader" />
	<include name = "VSM.shader" />
	<source>
	<!--

  #ifndef SHADOW_SHADER
  #define SHADOW_SHADER

  #define MAX_CASCADE_COUNT 4
  #define SHADOW_ATLAS_SIZE 2048.0

  bool geq(float a, float b)
  {
    // Large epsilon check, >= should be fine. Replace it with geq
	  return a - b >= -1.0;
  }

  /*
  * Given a shadow map size and start coordinates, finds the queried shadow map's layer and start coordinates.
  * Shadow maps for cascades and cube maps are placed sequentially to the atlas. So if you pass the directional
  * light's start location and start layer and query the layer and start coordinate of 4'th cascade, this function
  * will calculate it.
  */
  void ShadowAtlasLut(in float size, in vec2 startCoord, in int queriedMap, out int layer, out vec2 targetCoord)
  {
    vec2 area = vec2(SHADOW_ATLAS_SIZE);
    targetCoord = startCoord;
    layer = 0;
	
    for (int i = 1; i <= queriedMap; i++)
    {
	    targetCoord.x += size;

      if (geq(targetCoord.x, area.x))
      {
        targetCoord.x = 0.0;
        targetCoord.y += size;
        if (geq(targetCoord.y, area.y))
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
	vec2 sum = vec2(0.0);
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

#endif

	-->
	</source>
</shader>