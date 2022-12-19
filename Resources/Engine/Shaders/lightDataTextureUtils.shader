<shader>
	<type name = "includeShader" />
	<source>
	<!--
		
		const float LightTypeIndex = 0.0;
		const float DirColorIndex = 1.0;
		const float PointColorIndex = 1.0;
		const float SpotColorIndex = 1.0;
		const float DirIntensityIndex = 2.0;
		const float PointIntensityIndex = 2.0;
		const float SpotIntensityIndex = 2.0;
		const float DirDirectionIndex = 3.0;
		const float SpotDirectionIndex = 4.0;
		const float PointPositionIndex = 3.0;
		const float SpotPositionIndex = 3.0;
		const float PointRadiusIndex = 4.0;
		const float SpotRadiusIndex = 5.0;
		const float SpotOuterAngleIndex = 6.0;
		const float SpotInnerAngleIndex = 7.0;

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
			return ReadFloat(data, startingPoint + LightTypeIndex, lightDataTextureWidth);
		}

		vec3 DirLightColor(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadVec3(data, startingPoint + DirColorIndex, lightDataTextureWidth);
		}

		vec3 PointLightColor(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadVec3(data, startingPoint + PointColorIndex, lightDataTextureWidth);
		}

		vec3 SpotLightColor(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadVec3(data, startingPoint + SpotColorIndex, lightDataTextureWidth);
		}

		float DirLightIntensity(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadFloat(data, startingPoint + DirIntensityIndex, lightDataTextureWidth);
		}

		float PointLightIntensity(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadFloat(data, startingPoint + PointIntensityIndex, lightDataTextureWidth);
		}

		float SpotLightIntensity(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadFloat(data, startingPoint + SpotIntensityIndex, lightDataTextureWidth);
		}

		vec3 DirLightDirection(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadVec3(data, startingPoint + DirDirectionIndex, lightDataTextureWidth);
		}

		vec3 SpotLightDirection(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadVec3(data, startingPoint + SpotDirectionIndex, lightDataTextureWidth);
		}

		vec3 PointLightPosition(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadVec3(data, startingPoint + PointPositionIndex, lightDataTextureWidth);
		}

		vec3 SpotLightPosition(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadVec3(data, startingPoint + SpotPositionIndex, lightDataTextureWidth);
		}

		float PointLightRadius(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadFloat(data, startingPoint + PointRadiusIndex, lightDataTextureWidth);
		}

		float SpotLightRadius(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadFloat(data, startingPoint + SpotRadiusIndex, lightDataTextureWidth);
		}

		float SpotLightOuterAngle(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadFloat(data, startingPoint + SpotOuterAngleIndex, lightDataTextureWidth);
		}

		float SpotLightInnerAngle(sampler2D data, float startingPoint, float lightDataTextureWidth)
		{
			return ReadFloat(data, startingPoint + SpotInnerAngleIndex, lightDataTextureWidth);
		}

	-->
	</source>
</shader>