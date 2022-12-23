<shader>
	<type name = "includeShader" />
	<source>
	<!--
		vec2 ComputeMoments(float depth)
		{
			vec2 moments;
			moments.x = depth;
			float dx = dFdx(moments.x);
			float dy = dFdy(moments.x);
			moments.y = moments.x * moments.x + 0.25 * (dx * dx + dy * dy);
			return moments;
		}

		float linstep(float low, float high, float v)
		{
			return clamp((v - low)/(high - low), 0.0, 1.0);
		}

		float ChebyshevUpperBound(vec2 moments, float currFragDepth, float lightBleedRed, float shadowBias)
		{
			float p = step(currFragDepth, moments.x);
			float variance = max(moments.y - moments.x * moments.x, shadowBias);
			float d = currFragDepth - moments.x;
			float pMax = linstep(lightBleedRed, 1.0, variance/(variance + d * d));
			return min(max(p, pMax), 1.0);
		}
	-->
	</source>
</shader>
