<shader>
	<type name = "includeShader" />
	<include name = "pbr.shader" />
	<uniform name = "UseIbl" />
	<uniform name = "IblIntensity" />
	<uniform name = "IBLIrradianceMap" />
	<uniform name = "IblRotation" />
	<uniform name = "iblMaxReflectionLod" />
	<source>
	<!--
		uniform samplerCube s_texture7; 	// IBL Diffuse Map
		uniform samplerCube s_texture15; 	// IBL Pre-Filtered Specular Map
		uniform sampler2D s_texture16;		// IBL BRDF Lut

		uniform mat4 IblRotation;
		uniform int UseIbl;
		uniform float IblIntensity;
		uniform int iblMaxReflectionLod;

		vec3 IblDiffuse(vec3 normal)
		{
			vec3 irradiance = vec3(0.0);
			if (UseIbl == 1)
			{
				vec3 iblSamplerVec = (IblRotation * vec4(normal, 1.0)).xyz;
				vec3 iblIrradiance = texture(s_texture7, iblSamplerVec).rgb;
				irradiance = iblIrradiance * IblIntensity;
			}

			return irradiance;
		}

		vec3 IBLDiffusePBR(vec3 normal, vec3 fragToEye, vec3 albedo, float metallic, float roughness, vec3 fresnel)
		{
			vec3 irradiance = vec3(0.0);
			if (UseIbl == 1)
			{
				vec3 kS = fresnel;
				vec3 kD = 1.0 - kS;
				vec3 iblSamplerVec = (IblRotation * vec4(normal, 1.0)).xyz;
				vec3 iblIrradiance = texture(s_texture7, iblSamplerVec).rgb;
				vec3 diffuse    = iblIrradiance * albedo;
				irradiance    = kD * diffuse;
			}

			return irradiance;
		}

		vec3 IBLSpecularPBR(vec3 normal, vec3 fragToEye, float roughness, vec3 fresnel)
		{
			vec3 specular = vec3(0.0);
			if (UseIbl == 1)
			{
				vec3 R = reflect(-fragToEye, normal);
				vec3 iblSamplerVec = (IblRotation * vec4(R, 1.0)).xyz;
				float normalDotFragToEye = max(dot(normal, fragToEye), 0.0);

				vec3 preFilteredColor = textureLod(s_texture15, iblSamplerVec, roughness * float(iblMaxReflectionLod)).rgb;
				vec2 brdfFactor = texture(s_texture16, vec2(normalDotFragToEye, roughness)).rg;
				specular = preFilteredColor * (fresnel * brdfFactor.x + brdfFactor.y);
			}

			return specular;
		}

		vec3 IBLPBR(vec3 normal, vec3 fragToEye, vec3 albedo, float metallic, float roughness)
		{
			// Base reflectivity
			vec3 fresnel = BaseReflectivityPBR(vec3(0.04), albedo, metallic);
			fresnel = FresnelSchlickRoughness(max(dot(normal, fragToEye), 0.0), fresnel, roughness); 

			vec3 diffuse = IBLDiffusePBR(normal, fragToEye, albedo, metallic, roughness, fresnel);
			vec3 specular = IBLSpecularPBR(normal, fragToEye, roughness, fresnel);
			return (diffuse + specular) * IblIntensity;
		}
		
		// No specular, only diffuse
		vec3 IBLPhong(vec3 normal)
		{
			return IblDiffuse(normal) * IblIntensity;
		}
	-->
	</source>
</shader>