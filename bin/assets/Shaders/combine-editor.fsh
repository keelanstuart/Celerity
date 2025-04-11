uniform sampler2D uSamplerDiffuseMetalness;
uniform sampler2D uSamplerNormalAmbOcc;
uniform sampler2D uSamplerPosDepth;
uniform sampler2D uSamplerEmissionRoughness;
uniform sampler2D uSamplerEffectsColor;
uniform sampler2D uSamplerInterfaceColor;
uniform sampler2D uSamplerLights;
uniform sampler2D uSamplerShadow;
uniform sampler2D uSamplerAuxiliary;
uniform vec3 uSunDirection;
uniform vec3 uSunColor;
uniform vec3 uAmbientColor;
uniform mat4 uMatrixS;
uniform float uElapsedTime;
uniform vec3 uEyePosition;
uniform vec3 uEyeDirection;

in vec2 fTex0;

layout (location=0) out vec4 oColor;

vec3 acquireAdjacentNormal(sampler2D samp, vec2 uv, vec2 ofs, vec3 def)
{
	vec3 t = texture(samp, uv + ofs).rgb;
	if (t == vec3(0, 0, 0))
		return def;

	return normalize(t * 2.0 - 1.0);
}

void main()
{
	vec4 texNormalAmbOcc = texture(uSamplerNormalAmbOcc, fTex0);
	vec4 texInterfaceColor = texture(uSamplerInterfaceColor, fTex0);
	
	ivec2 aux_tex_size = textureSize(uSamplerAuxiliary, 0);
	vec2 aux_texel_inc;
	aux_texel_inc.x = 1.0 / float(aux_tex_size.x) * 4.0;
	aux_texel_inc.y = 1.0 / float(aux_tex_size.y) * 4.0;
	
	vec4 aux[9];
	aux[0] = texture(uSamplerAuxiliary, vec2(fTex0.x - aux_texel_inc.x,	fTex0.y - aux_texel_inc.y));
	aux[1] = texture(uSamplerAuxiliary, vec2(fTex0.x,					fTex0.y - aux_texel_inc.y));
	aux[2] = texture(uSamplerAuxiliary, vec2(fTex0.x + aux_texel_inc.x,	fTex0.y - aux_texel_inc.y));
	aux[3] = texture(uSamplerAuxiliary, vec2(fTex0.x - aux_texel_inc.x,	fTex0.y));
	aux[4] = texture(uSamplerAuxiliary, vec2(fTex0.x,					fTex0.y));
	aux[5] = texture(uSamplerAuxiliary, vec2(fTex0.x + aux_texel_inc.x,	fTex0.y));
	aux[6] = texture(uSamplerAuxiliary, vec2(fTex0.x - aux_texel_inc.x,	fTex0.y + aux_texel_inc.y));
	aux[7] = texture(uSamplerAuxiliary, vec2(fTex0.x,					fTex0.y + aux_texel_inc.y));
	aux[8] = texture(uSamplerAuxiliary, vec2(fTex0.x + aux_texel_inc.x,	fTex0.y + aux_texel_inc.y));

	vec2 aux_avg = vec2(0, 0);
	for (int i = 0; i < 9; i++)
	{
		aux_avg += aux[i].rg;
	}
	aux_avg /= 9.0;

	vec4 texPositionDepth = texture(uSamplerPosDepth, fTex0);
	vec4 texDiffuseMetalness = texture(uSamplerDiffuseMetalness, fTex0);
	vec4 texEmissiveRoughness = texture(uSamplerEmissionRoughness, fTex0);
	vec3 emissive = texEmissiveRoughness.rgb;

	vec3 glow_color = vec3((1.0 - aux[4].r) * sin(aux_avg.r * 1.5707) * 2.0);

	float cloud_depth = max(0, min(aux[4].a, texPositionDepth.a) - aux[4].b);
	// replace cloud_max and cloud_color with uniforms
	float cloud_max = 100.0;
	vec3 cloud_color = vec3(1, 1, 1);
	float cloud_factor = clamp(cloud_depth / cloud_max, 0, 1);
	vec3 cloudiness = cloud_color * cloud_factor;

	vec4 texEffectsColor = texture(uSamplerEffectsColor, fTex0);

	vec3 cc;

	if ( (texNormalAmbOcc.rgb == vec3(0, 0, 0)) || (texNormalAmbOcc.rgb == vec3(1, 1, 1)) )
	{
		cc = texDiffuseMetalness.rgb + texEffectsColor.rgb + cloudiness + glow_color + (emissive * (1 - cloud_factor));
		oColor = vec4(mix(cc, texInterfaceColor.rgb, texInterfaceColor.a), 1);
		return;
	}
	
	vec3 norm = normalize(texNormalAmbOcc.rgb * 2.0 - 1.0);

	vec3 sunlight = normalize(-uSunDirection);
	float NdotL = max(dot(norm, sunlight), 0.0);
	
	vec3 lights = texture(uSamplerLights, fTex0).rgb * texDiffuseMetalness.rgb;
	
	vec3 diffuse = texDiffuseMetalness.rgb * uSunColor;
	vec3 ambient = texDiffuseMetalness.rgb * uAmbientColor;
	float metalness = texDiffuseMetalness.a;
	float roughness = texEmissiveRoughness.a;
	
	vec3 view = normalize(texPositionDepth.xyz - uEyePosition);
	vec3 refl = normalize(-reflect(-sunlight, norm));
	float spec = pow(max(dot(view, refl), 0.0), roughness * 16.0);
	vec3 specular = spec * mix(uSunColor, texDiffuseMetalness.rgb, metalness) * 0.4;
	
	vec4 shadow_pos = uMatrixS * vec4(texPositionDepth.xyz, 1.0);
	vec3 shadow_coords = (shadow_pos.xyz / shadow_pos.w) * 0.5 + 0.5;

	float shadow = texture(uSamplerShadow, shadow_coords.xy).r;

	float bias = clamp(0.005 * tan(acos(NdotL)), 0.0, 0.01);  
	
	float shade = (shadow_coords.z > (shadow + bias)) ? 0.0 : 1.0;

	cc = ((mix(ambient, diffuse, NdotL * shade) + ((specular + cloudiness) * shade))) + (emissive * (1 - cloud_factor)) + lights + glow_color + texEffectsColor.rgb;
	oColor = vec4(mix(cc, texInterfaceColor.rgb, texInterfaceColor.a), 1);
}
