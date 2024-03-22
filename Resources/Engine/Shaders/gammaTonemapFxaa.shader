<shader>
	<type name = "fragmentShader" />
  <include name = "fxaaFunctions.shader" />
  <include name = "tonemapFunctions.shader" />
  <inlcude name = "fxaaFunctions.shader" />
  <include name = "gammaFunctions.shader" />
	<source>
	<!--
		#version 300 es
		precision highp float;
		
    uniform int enableFxaa;
    uniform int enableTonemapping;
    uniform int enableGammaCorrection;

		uniform sampler2D s_texture0;
		in vec2 v_texture;

		out vec4 fragColor;

		void main()
		{
			vec2 uv = vec2(v_texture.x, 1.0 - v_texture.y);

      if (enableFxaa != 0)
      {
        fragColor = Fxaa(uv, s_texture0);
      }
      else
      {
        fragColor = texture(s_texture0, uv);
      }

      if (enableTonemapping != 0)
      {
        fragColor.rgb = Tonemap(fragColor.rgb);
      }

      if (enableGammaCorrection != 0)
      {
        fragColor.rgb = Gamma(fragColor.rgb);
      }
		}
	-->
	</source>
</shader>