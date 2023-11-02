<shader>
	<type name = "includeShader" />
	<include name = "lighting.shader" />

	<source>
	<!--

  uniform vec3  lightPos;
  uniform vec3  lightDir;
  uniform vec3  lightColor;
  uniform float lightIntensity;
  uniform float lightRadius;
  uniform float lightOutAngle;
  uniform float lightInnAngle;
  uniform mat4  lightProjectionViewMatrix;
  uniform float lightShadowMapCameraFar;
  uniform int   lightCastShadow;
  uniform int   lightPCFSamples;
  uniform float lightPCFRadius;
  uniform float lightBleedReduction;
  uniform float lightShadowAtlasLayer;
  uniform float lightShadowAtlasResRatio;
  uniform vec2  lightShadowAtlasCoord; // Between 0 and 1
  uniform float lightShadowBias;
  
  /*
    Type
    shadowDirLights
    shadowPointLights
    shadowSpotLights
    nonShadowDirLights
    nonShadowPointLights
    nonShadowSpotLights
    */
  uniform int lightType;
  
  vec3 AdditivePBRLighting(vec3 fragPos, vec3 normal, vec3 fragToEye, vec3 viewCamPos, vec3 albedo, float metallic, float roughness)
  {
    float shadow = 1.0;
    vec3 irradiance = vec3(0.0);
    vec3 color = lightColor * lightIntensity;
  
    switch (lightType)
    {
      case 3: // directional with shadows
      {
        shadow = CalculateDirectionalShadow(fragPos, viewCamPos, lightProjectionViewMatrix, lightShadowAtlasCoord, lightShadowAtlasResRatio,
                                            lightShadowAtlasLayer, lightPCFSamples, lightPCFRadius, 
                                            lightBleedReduction, lightShadowBias);
      } // fallthrough
      case 0: // directional light
      {
        // lighting
        vec3 Lo = PBR(fragPos, normal, fragToEye, albedo, metallic, roughness, -lightDir, color);
        irradiance += Lo * shadow;
      } 
      break;
      case 4: // point with shadows
      {
        shadow = CalculatePointShadow(fragPos, lightPos, lightShadowMapCameraFar, lightShadowAtlasCoord, lightShadowAtlasResRatio,
                                      lightShadowAtlasLayer, lightPCFSamples, lightPCFRadius, 
                                      lightBleedReduction, lightShadowBias);
      } // fallthrough
      case 1: // point light
      {
        // radius check and attenuation
        float lightDistance = length(lightPos - fragPos);
        float radiusCheck = RadiusCheck(lightRadius, lightDistance);
        float attenuation = Attenuation(lightDistance, lightRadius, 1.0, 0.09, 0.032);
        // lighting
        vec3 lightDir = normalize(lightPos - fragPos);
        vec3 Lo = PBR(fragPos, normal, fragToEye, albedo, metallic, roughness, lightDir, color);
        irradiance += Lo * shadow * attenuation * radiusCheck;
      }
      break;
      case 5: // spot with shadows
      {
        shadow = CalculateSpotShadow(fragPos, lightPos, lightProjectionViewMatrix, lightShadowMapCameraFar, lightShadowAtlasCoord,
                                    lightShadowAtlasResRatio, lightShadowAtlasLayer, lightPCFSamples, 
                                    lightPCFRadius, lightBleedReduction, lightShadowBias);
      } // fallthrough
      case 2: // spot light
      {
        // radius check and attenuation
        vec3 fragToLight    = lightPos - fragPos;
        float lightDistance = length(fragToLight);
        float radiusCheck   = RadiusCheck(lightRadius, lightDistance);
        float attenuation   = Attenuation(lightDistance, lightRadius, 1.0, 0.09, 0.032);
        // Lighting angle and falloff
        float theta     = dot(-normalize(fragToLight), lightDir);
        float epsilon   = lightInnAngle - lightOutAngle;
        float intensity = clamp((theta - lightOutAngle) / epsilon, 0.0, 1.0);
        // lighting
        vec3 Lo = PBR(fragPos, normal, fragToEye, albedo, metallic, roughness, -lightDir, color);
        irradiance += Lo * shadow * intensity * radiusCheck * attenuation;
      }
      break;
    }

    return irradiance;
  }
	-->
	</source>
</shader>