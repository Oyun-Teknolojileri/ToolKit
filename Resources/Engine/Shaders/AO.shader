<shader>
	<type name = "includeShader" />
	<uniform name = "UseAO" />
	<source>
	<!--
		uniform sampler2D s_texture5;

		uniform int UseAO;

		float AmbientOcclusion()
		{
			vec2 coords = gl_FragCoord.xy / vec2(textureSize(s_texture5, 0));
			coords = vec2(coords.x, coords.y);

			float ambientOcclusion;
			if (UseAO == 0)
			{
				ambientOcclusion = 1.0;
			}
			else
			{
				ambientOcclusion = texture(s_texture5, coords).r;
			}

			return ambientOcclusion;
		}
	-->
	</source>
</shader>