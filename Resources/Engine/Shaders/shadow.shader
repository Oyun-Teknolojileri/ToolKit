<shader>
	<type name = "includeShader" />
	<include name = "VSM.shader" />
  <uniform name = "shadowAtlasSize" />
	<source>
	<!--

  #ifndef SHADOW_SHADER
  #define SHADOW_SHADER

  uniform float shadowAtlasSize;

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
  vec2 startCoord[6],
  float shadowAtlasResRatio,
  float shadowAtlasLayer[6],
  vec3 dir,
  int samples,
  float radius,
  float currDepth,
  float LBR,
  float shadowBias
)
{
  vec2 halfPixel = vec2((1.0 / shadowAtlasSize) * 0.5);

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

    vec2 beginCoord = startCoord[face];
    vec2 endCoord = beginCoord + shadowAtlasResRatio;

    texCoord.xy = beginCoord + (shadowAtlasResRatio * texCoord.xy);
    texCoord.z = shadowAtlasLayer[face];

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