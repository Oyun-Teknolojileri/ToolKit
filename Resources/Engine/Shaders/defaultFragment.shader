<shader>
	<type name = "fragmentShader" />
	<include name = "lighting.shader" />
	<include name = "ibl.shader" />
	<include name = "camera.shader" />
	<include name = "AO.shader" />
	<uniform name = "alphaMaskTreshold" />
	<uniform name = "DiffuseTextureInUse" />
	<uniform name = "Color" />
	<uniform name = "emissiveColor" />
	<uniform name = "emissiveTextureInUse" />
	<uniform name = "metallicRoughnessTextureInUse" />
	<uniform name = "metallic" />
	<uniform name = "roughness" />
  <uniform name = "normalMapInUse" />
	<define name = "DrawAlphaMasked" val="0,1" />
	<source>
	<!--
#version 300 es
precision highp float;
precision mediump sampler2D;
precision mediump samplerCube;
precision highp sampler2DArray;
precision lowp int;

uniform sampler2D s_texture0; // color
uniform sampler2D s_texture1; // emissive
uniform sampler2D s_texture4; // metallic-roughness
uniform sampler2D s_texture9; // normal

uniform int LightingOnly;
uniform float alphaMaskTreshold;
uniform int DiffuseTextureInUse;
uniform vec4 Color;
uniform int emissiveTextureInUse;
uniform vec3 emissiveColor;

uniform int metallicRoughnessTextureInUse;
uniform float metallic;
uniform float roughness;

uniform int normalMapInUse;

in vec3 v_pos;
in vec3 v_normal;
in vec2 v_texture;
in float v_viewPosDepth;
in mat3 TBN;

layout (location = 0) out vec4 fragColor;

void main()
{
	vec4 color;
	if(DiffuseTextureInUse > 0)
	{
		color = texture(s_texture0, v_texture);
	}
	else
	{
		color = Color;
	}
	
	vec3 emissive;
	if(emissiveTextureInUse > 0)
	{
		emissive = texture(s_texture1, v_texture).xyz;
	}
	else
	{
		emissive = emissiveColor;
	}

#if DrawAlphaMasked
	if(color.a <= alphaMaskTreshold)
	{
		discard;
	}
#endif

	if (LightingOnly == 1)
	{
		color.xyz = vec3(1.0);
	}

	vec3 n;
	if (normalMapInUse == 1)
	{
		n = texture(s_texture9, v_texture).xyz;
		n = n * 2.0 - 1.0;
		n = TBN * n;
		n = normalize(n);
	}
	else
	{
		n = normalize(v_normal);
	}
	vec3 e = normalize(CamData.pos - v_pos);

	vec2 metallicRoughness;
	if (metallicRoughnessTextureInUse == 1)
	{
		metallicRoughness = texture(s_texture4, v_texture).rg;
	}
	else
	{
		metallicRoughness = vec2(metallic, roughness);
	}

	vec3 irradiance = PBRLighting(v_pos, v_viewPosDepth, n, e, CamData.pos, color.xyz, metallicRoughness.x, metallicRoughness.y);

	irradiance += IBLPBR(n, e, color.xyz, metallicRoughness.x, metallicRoughness.y);

	float ambientOcclusion = AmbientOcclusion();
	fragColor = vec4(irradiance * ambientOcclusion, color.a) + vec4(emissive, 0.0f);
}
	-->
	</source>
</shader>