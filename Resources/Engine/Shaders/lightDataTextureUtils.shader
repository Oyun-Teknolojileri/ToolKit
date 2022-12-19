<shader>
	<type name = "includeShader" />
	<source>
	<!--

		float ReadFloat(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			vec2 loc = vec2(mod(startingPoint, lightDataTextureWidth), floor(startingPoint / lightDataTextureWidth));
			loc = loc / lightDataTextureWidth; // TODO Can be done in CPU
			float type = texture(data, loc).r;
			return type;
		}

		vec3 ReadVec3(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			vec2 loc = vec2(mod(startingPoint, lightDataTextureWidth), floor(startingPoint / lightDataTextureWidth));
			loc = loc / lightDataTextureWidth; // TODO Can be done in CPU
			vec3 type = texture(data, loc).rgb;
			return type;
		}

		float LightType(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadFloat(data, startingPoint + 0.0, lightDataTextureWidth);
		}

		vec3 DirLightColor(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadVec3(data, startingPoint + 1.0, lightDataTextureWidth);
		}

		vec3 PointLightColor(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadVec3(data, startingPoint + 1.0, lightDataTextureWidth);
		}

		vec3 SpotLightColor(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadVec3(data, startingPoint + 1.0, lightDataTextureWidth);
		}

		float DirLightIntensity(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadFloat(data, startingPoint + 2.0, lightDataTextureWidth);
		}

		float PointLightIntensity(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadFloat(data, startingPoint + 2.0, lightDataTextureWidth);
		}

		float SpotLightIntensity(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadFloat(data, startingPoint + 2.0, lightDataTextureWidth);
		}

		vec3 DirLightDirection(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadVec3(data, startingPoint + 3.0, lightDataTextureWidth);
		}

		vec3 SpotLightDirection(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadVec3(data, startingPoint + 4.0, lightDataTextureWidth);
		}

		vec3 PointLightPosition(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadVec3(data, startingPoint + 3.0, lightDataTextureWidth);
		}

		vec3 SpotLightPosition(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadVec3(data, startingPoint + 3.0, lightDataTextureWidth);
		}

		float PointLightRadius(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadFloat(data, startingPoint + 4.0, lightDataTextureWidth);
		}

		float SpotLightRadius(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadFloat(data, startingPoint + 5.0, lightDataTextureWidth);
		}

		float SpotLightOuterAngle(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadFloat(data, startingPoint + 6.0, lightDataTextureWidth);
		}

		float SpotLightInnerAngle(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadFloat(data, startingPoint + 7.0, lightDataTextureWidth);
		}

	-->
	</source>
</shader>