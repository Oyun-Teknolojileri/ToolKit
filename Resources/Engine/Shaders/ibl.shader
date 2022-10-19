<shader>
	<type name = "includeShader" />
	<uniform name = "UseIbl" />
	<uniform name = "IblIntensity" />
	<uniform name = "IBLIrradianceMap" />
	<uniform name = "IblRotation" />
	<source>
	<!--
		uniform samplerCube s_texture7;

		uniform mat4 IblRotation;
		uniform float UseIbl;
		uniform float IblIntensity;

		vec3 IblIrradiance(vec3 normal)
		{
			vec3 iblSamplerVec = (IblRotation * vec4(normal, 1.0)).xyz;
			vec3 iblIrradiance = UseIbl * texture(s_texture7, iblSamplerVec).rgb;
			return iblIrradiance * IblIntensity;
		}
	-->
	</source>
</shader>