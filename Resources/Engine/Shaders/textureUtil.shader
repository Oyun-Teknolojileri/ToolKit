<shader>
	<type name = "includeShader" />
	<source>
	<!--	
		// TODO there should be a better alogrithm for this
		// This shows artifacts in low resolutions (lower than 100x100)
		// Maybe this? https://github.com/turanszkij/WickedEngine/blob/master/WickedEngine/shaders/lightingHF.hlsli#L87
		vec2 ClampTextureCoordinates(vec2 coord, vec2 mn, vec2 mx)
		{
			return max(min(coord, mx - 0.00015), mn);
		}
	-->
	</source>
</shader>
