<shader>
	<type name = "includeShader" />
	<uniform name = "CamData.pos" />
	<uniform name = "CamData.dir" />
	<uniform name = "CamData.far" />
	<source>
	<!--
		struct _CamData
		{
			vec3 pos;
			vec3 dir;
			float far;
		};

		uniform _CamData CamData;
	-->
	</source>
</shader>