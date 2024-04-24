<shader>
	<type name = "includeShader" />
	<include name = "VSM.shader" />
	<source>
	<!--
		const vec3 poissonDisk[128] = vec3[](
        vec3(-0.308466, -0.140553, -0.393857),
        vec3(-0.422605, 0.380276, -0.49527),
        vec3(0.446471, 0.0725272, -0.292199),
        vec3(-0.346095, -0.128437, 0.212912),
        vec3(0.186911, -0.471374, -0.118),
        vec3(-0.0681326, 0.347896, 0.0368816),
        vec3(-0.488922, 0.447325, 0.324915),
        vec3(0.228751, 0.05974, 0.331782),
        vec3(0.151418, 0.443449, -0.48233),
        vec3(0.417875, -0.463469, 0.418912),
        vec3(0.229453, 0.412809, 0.475585),
        vec3(0.0848262, -0.0337077, -0.268303),
        vec3(0.375729, -0.330988, -0.458647),
        vec3(-0.165731, 0.207511, 0.427885),
        vec3(-0.0748772, -0.282098, 0.440733),
        vec3(0.366756, 0.323298, 0.0856197),
        vec3(-0.352474, -0.459594, -0.0412152),
        vec3(-0.471618, 0.331172, -0.117267),
        vec3(-0.0880612, -0.474059, -0.398312),
        vec3(-0.424589, -0.461272, 0.460021),
        vec3(0.443144, -0.219077, 0.053209),
        vec3(-0.136586, -0.0444807, -0.0303812),
        vec3(0.241447, 0.294549, -0.183523),
        vec3(-0.158986, 0.159017, -0.370052),
        vec3(-0.473785, 0.0657827, 0.48352),
        vec3(-0.0282144, -0.499329, 0.217582),
        vec3(0.432737, 0.214133, 0.465697),
        vec3(-0.488433, -0.0505844, -0.0291604),
        vec3(0.476806, 0.467803, -0.202506),
        vec3(-0.126484, 0.452147, -0.287317),
        vec3(0.491119, -0.0223243, 0.252647),
        vec3(-0.366604, 0.268212, 0.137043),
        vec3(0.0865352, -0.168172, 0.19512),
        vec3(0.286248, 0.0381939, 0.00587481),
        vec3(0.226096, 0.146748, -0.476928),
        vec3(0.327754, -0.199179, -0.212943),
        vec3(-0.163686, 0.470031, 0.292352),
        vec3(0.499207, 0.496063, 0.362087),
        vec3(-0.425291, -0.47055, -0.336024),
        vec3(0.214774, -0.267266, 0.434812),
        vec3(-0.100879, -0.33578, -0.131336),
        vec3(0.261559, -0.498413, 0.186239),
        vec3(0.488433, -0.470428, -0.0959044),
        vec3(0.110187, -0.214866, -0.495819),
        vec3(-0.435514, 0.044145, -0.459624),
        vec3(-0.237175, -0.434416, 0.336329),
        vec3(0.127827, -0.290002, -0.0234535),
        vec3(-0.496948, -0.360286, 0.199149),
        vec3(-0.0235755, 0.164205, 0.15746),
        vec3(-0.351863, 0.347835, 0.493805),
        vec3(0.483642, -0.142537, 0.429136),
        vec3(-0.341365, 0.103992, -0.229698),
        vec3(-0.335444, 0.310419, -0.292199),
        vec3(0.135823, -0.419401, -0.445036),
        vec3(-0.160756, -0.261254, 0.152089),
        vec3(0.0754265, 0.433866, -0.191824),
        vec3(-0.193503, 0.0736259, 0.264977),
        vec3(0.0212867, 0.316919, 0.39758),
        vec3(-0.0430464, 0.246422, -0.146596),
        vec3(-0.122578, -0.0722221, -0.255028),
        vec3(0.00968963, -0.289575, -0.320734),
        vec3(0.0941954, 0.453398, 0.137562),
        vec3(-0.483489, 0.115467, 0.259728),
        vec3(-0.499634, -0.232109, 0.426572),
        vec3(-0.00840786, -0.0442976, 0.460021),
        vec3(0.283654, 0.494354, -0.120746),
        vec3(-0.259972, 0.465056, 0.0141148),
        vec3(-0.283563, -0.2875, 0.494995),
        vec3(-0.0184484, 0.285607, -0.472869),
        vec3(-0.483062, -0.256462, -0.360378),
        vec3(0.145711, 0.207297, 0.024369),
        vec3(0.276116, -0.281487, 0.204611),
        vec3(0.26339, 0.48468, 0.264336),
        vec3(0.430265, 0.29397, -0.34405),
        vec3(0.354396, -0.0206458, -0.498016),
        vec3(-0.254479, 0.00447094, 0.471923),
        vec3(-0.335505, -0.202841, -0.143696),
        vec3(0.128254, -0.484283, 0.474792),
        vec3(0.464965, -0.45822, 0.138203),
        vec3(0.405759, 0.472137, -0.484405),
        vec3(0.448912, 0.24572, -0.0946837),
        vec3(0.0824152, -0.0541551, -0.0445113),
        vec3(0.360164, -0.466063, -0.264855),
        vec3(-7.62939e-05, 0.0322734, -0.456206),
        vec3(-0.254295, 0.254204, -0.0641347),
        vec3(-0.443205, 0.499298, -0.291864),
        vec3(-0.473571, 0.46057, 0.0775018),
        vec3(0.468322, 0.18511, 0.217856),
        vec3(-0.216575, -0.490875, 0.121632),
        vec3(0.476043, -0.0775628, -0.117359),
        vec3(-0.175039, 0.169851, -0.200797),
        vec3(-0.255333, 0.401914, -0.178488),
        vec3(0.327906, 0.483093, -0.32165),
        vec3(0.0749687, 0.192984, -0.38461),
        vec3(0.103412, 0.236015, 0.220481),
        vec3(-0.310694, -0.226096, 0.0920286),
        vec3(0.315546, -0.191031, -0.0405744),
        vec3(-0.281701, 0.479919, -0.372433),
        vec3(-0.323725, 0.46704, 0.223319),
        vec3(0.242363, 0.0209205, -0.257744),
        vec3(-0.207297, -0.298578, -0.494842),
        vec3(-0.492187, -0.281549, -0.176473),
        vec3(0.122547, -0.45294, 0.0645924),
        vec3(-0.287927, 0.234214, -0.470855),
        vec3(-0.0479904, -0.271355, 0.280145),
        vec3(-0.117298, -0.0740227, 0.16216),
        vec3(0.141407, -0.341548, 0.273888),
        vec3(-0.195334, -0.274743, -0.27926),
        vec3(0.3081, -0.473479, -0.430631),
        vec3(-0.377407, -0.0253151, -0.141285),
        vec3(-0.116657, 0.486724, 0.141499),
        vec3(0.472594, -0.200461, 0.241295),
        vec3(-0.192343, 0.269585, 0.11446),
        vec3(0.382412, -0.265252, 0.355647),
        vec3(0.0450606, 0.126606, 0.486572),
        vec3(-0.313837, 0.0834834, 0.0608081),
        vec3(0.161122, -0.189016, -0.317682),
        vec3(0.0235145, 0.498993, 0.452025),
        vec3(-0.0971862, -0.447417, 0.443632),
        vec3(0.168844, -0.0318156, -0.444792),
        vec3(0.448393, 0.0395672, 0.436888),
        vec3(-0.484527, -0.444761, -0.164876),
        vec3(-0.473846, 0.198233, -0.472686),
        vec3(0.437223, 0.477386, 0.0479904),
        vec3(-0.0225684, 0.469573, -0.451384),
        vec3(-0.368313, -0.363735, -0.455809),
        vec3(0.233665, -0.114032, 0.242119),
        vec3(-0.209922, -0.437284, -0.235557)
    );

    // NOTE: "ClampTextureCoordinates" function is from "textureUtil.shader" and this shader includes from "lighting.shader" which already includes that.
    // If you need to use that function from your shader, include "textureUtil.shader".

    // Uses variance shadow map
		float PCFFilterShadow2D(sampler2DArray shadowAtlas, vec3 uvLayer, vec2 coordStart, vec2 coordEnd, int samples, float radius, float currDepth, float LBR, float shadowBias)
		{
			float sum = 0.0;
			for (int i = 0; i < samples; ++i)
			{
				vec2 offset = poissonDisk[i].xy * radius;
        vec3 texCoord = uvLayer;
        texCoord.xy = ClampTextureCoordinates(uvLayer.xy + offset, coordStart, coordEnd);
				vec2 moments = texture(shadowAtlas, texCoord).xy;
				sum += ChebyshevUpperBound(moments, currDepth, LBR, shadowBias);
			}
			return sum / float(samples);
		}

    // Uses depth sample shadow map
    float PCFFilter(sampler2DArray shadowAtlas, vec3 uvLayer, vec2 coordStart, vec2 coordEnd, int samples, float radius, float currDepth, float shadowBias)
    {
			float sum = 0.0;
			for (int i = 0; i < samples; ++i)
			{
				vec2 offset = poissonDisk[i].xy * radius;
        vec3 texCoord = uvLayer;
        texCoord.xy = ClampTextureCoordinates(uvLayer.xy + offset, coordStart, coordEnd);
        float smDepth = texture(shadowAtlas, texCoord).x;
        sum += smDepth + shadowBias < currDepth ? 0.0 : 1.0;
			}
			return sum / float(samples);
    }

    float PCFFilterOmni(sampler2DArray shadowAtlas, vec2 startCoord, float shadowAtlasResRatio, float shadowAtlasLayer,
    vec3 dir, int samples, float radius, float currDepth, float shadowBias)
    {
			float sum = 0.0;
			for (int i = 0; i < samples; ++i)
			{
				vec3 offset = poissonDisk[i] * radius;
        vec3 sampleDir = dir + offset;
        vec3 coord = UVWToUVLayer(sampleDir);
        coord.xy = startCoord + shadowAtlasResRatio * coord.xy;
	      coord.z = shadowAtlasLayer + coord.z;
        float smDepth = texture(shadowAtlas, coord).x;
        sum += smDepth + shadowBias < currDepth ? 0.0 : 1.0;
			}
			return sum / float(samples);
    }

    float ShadowBiasCalc(float bias, vec3 lightDir, vec3 normal)
    {
      float newBias = max(bias * (1.0 - dot(normal, lightDir)), 0.005);
      return newBias;
    }
	-->
	</source>
</shader>