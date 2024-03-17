<shader>
	<type name = "fragmentShader" />
	<include name = "AO.shader" />
	<source>
	<!--
		#version 300 es
		precision highp float;
		precision highp sampler2D;
		precision highp samplerCube;
		precision highp sampler2DArray;

		uniform sampler2D s_texture0;  // Irradiance
		uniform sampler2D s_texture1;  // Emmisive 
		uniform sampler2D s_texture2;  // ibl

		in vec2 v_texture;
		out vec4 fragColor;
		
		void main()
		{
			vec2 texCoord   = vec2(v_texture.x, 1.0 - v_texture.y);
			vec3 irradiance = texture(s_texture0, texCoord).rgb;
			vec3 emmisive   = texture(s_texture1, texCoord).rgb;
			vec3 ibl        = texture(s_texture2, texCoord).rgb;
			vec3 color      = (irradiance + ibl) * AmbientOcclusion(); 
			fragColor = vec4(color, 1.0) + vec4(emmisive, 0.0f);
		}
	-->
	</source>
</shader>