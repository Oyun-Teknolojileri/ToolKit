<shader>
	<type name = "fragmentShader" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform nam = "ColorAlpha" />
	<source>
	<!--
		#version 300 es
		precision mediump float;

		in vec3 v_normal;
		in vec2 v_texture;

		uniform int diffuseTextureInUse;
		uniform sampler2D s_texture0;
		uniform float colorAlpha;

		void main()
		{
			float alpha = float(1 - diffuseTextureInUse) * colorAlpha
				+ float(diffuseTextureInUse) * texture(s_texture0, v_texture).a;
			if (alpha < 0.1)
			{
				discard;
			}

			//gl_FragDepth = gl_FragCoord.z;
		}
	-->
	</source>
</shader>