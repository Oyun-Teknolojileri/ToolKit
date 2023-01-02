<shader>
	<type name = "includeShader" />
	<include name = "pbr.shader" />
	<uniform name = "UseIbl" />
	<uniform name = "IblIntensity" />
	<uniform name = "IBLIrradianceMap" />
	<uniform name = "IblRotation" />
	<source>
	<!--
		uniform samplerCube s_texture7;

		uniform mat4 IblRotation;
		uniform int UseIbl;
		uniform float IblIntensity;

		vec3 IblIrradiance(vec3 normal)
		{
			vec3 irradiance = vec3(0.0);
			if (UseIbl == 1)
			{
				vec3 iblSamplerVec = (IblRotation * vec4(normal, 1.0)).xyz;
				vec3 iblIrradiance = texture(s_texture7, iblSamplerVec).rgb;
				irradiance = iblIrradiance * IblIntensity;
			}

			return irradiance * IblIntensity;
		}

		vec3 IBLIrradiancePBR(vec3 normal, vec3 fragToEye, vec3 albedo, float metallic, float roughness)
		{
			vec3 irradiance = vec3(0.0);
			if (UseIbl == 1)
			{
				// Base reflectivity
				vec3 F0 = BaseReflectivityPBR(vec3(0.04), albedo, metallic);

				vec3 kS = FresnelSchlickRoughness(max(dot(normal, fragToEye), 0.0), F0, roughness); 
				vec3 kD = 1.0 - kS;
				vec3 iblIrradiance = texture(s_texture7, normal).rgb;
				vec3 diffuse    = iblIrradiance * albedo;
				irradiance    = kD * diffuse;
			}

			return irradiance * IblIntensity;
		}
	-->
	</source>
</shader>