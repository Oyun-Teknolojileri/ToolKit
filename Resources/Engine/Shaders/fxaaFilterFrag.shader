<shader>
	<type name = "fragmentShader" />
	<source>
	<!--
		#version 300 es
		precision highp float;
		
		uniform sampler2D s_texture0;
		in vec2 v_texture;
		
		out vec4 fragColor;

		uniform vec2 screen_size;

// https://www.shadertoy.com/view/4tf3D8

#define RES screen_size.xy

vec3 fxaa(vec2 p)
{
	float FXAA_SPAN_MAX   = 8.0;
    float FXAA_REDUCE_MUL = 1.0 / 8.0;
    float FXAA_REDUCE_MIN = 1.0 / 128.0;

    // 1st stage - Find edge
    vec3 rgbNW = texture(s_texture0, p + (vec2(-1.,-1.) / RES)).rgb;
    vec3 rgbNE = texture(s_texture0, p + (vec2( 1.,-1.) / RES)).rgb;
    vec3 rgbSW = texture(s_texture0, p + (vec2(-1., 1.) / RES)).rgb;
    vec3 rgbSE = texture(s_texture0, p + (vec2( 1., 1.) / RES)).rgb;
    vec3 rgbM  = texture(s_texture0, p).rgb;

    vec3 luma = vec3(0.299, 0.587, 0.114);

    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
    
    float lumaSum   = lumaNW + lumaNE + lumaSW + lumaSE;
    float dirReduce = max(lumaSum * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    float rcpDirMin = 1. / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2(FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX), dir * rcpDirMin)) / RES;

    // 2nd stage - Blur
    vec3 rgbA = .5 * (texture(s_texture0, p + dir * (1./3. - .5)).rgb +
        			  texture(s_texture0, p + dir * (2./3. - .5)).rgb);
    vec3 rgbB = rgbA * .5 + .25 * (
        			  texture(s_texture0, p + dir * (0./3. - .5)).rgb +
        			  texture(s_texture0, p + dir * (3./3. - .5)).rgb);
    
    float lumaB = dot(rgbB, luma);
    
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    return ((lumaB < lumaMin) || (lumaB > lumaMax)) ? rgbA : rgbB;
}

void main()
{
	vec2 uv = vec2(v_texture.x, 1.0 - v_texture.y);

	fragColor = vec4(fxaa(uv), 1);

    // Split screen on/off for debugging purpose
	/*if (uv.x < 0.5)
		fragColor = texture(s_texture0, uv);

	if (uv.x > 0.498 && uv.x < 0.502 )
		fragColor = vec4(1.0, 0.0, 0.0, 1.0);*/
}
	-->
	</source>
</shader>
