<shader>
	<type name = "includeShader" />
	<source>
	<!--
		uniform sampler2D s_texture5;
		float AmbientOcclusion()
		{
			vec2 coords = gl_FragCoord.xy / vec2(textureSize(s_texture5, 0));
			coords = vec2(coords.x, coords.y);
			return texture(s_texture5, coords).r;;
		}
	-->
	</source>
</shader>