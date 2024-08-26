uniform sampler2D uSamplerDiffuseMetalness;
uniform sampler2D uSamplerNormalAmbOcc;
uniform sampler2D uSamplerPosDepth;
uniform sampler2D uSamplerEmissionRoughness;
uniform sampler2D uSamplerLights;
uniform sampler2D uSamplerShadow;
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

	return normalize(t - 0.5 * 2.0);
}

void main()
{
	vec4 texNormalAmbOcc = texture(uSamplerNormalAmbOcc, fTex0);
	
	
	vec4 texDiffuseMetalness = texture(uSamplerDiffuseMetalness, fTex0);
	if (texNormalAmbOcc.rgb == vec3(0, 0, 0))
	{
		oColor = vec4(texDiffuseMetalness.rgb, 1);
		return;
	}
	
	vec3 norm = normalize((normalize(texNormalAmbOcc.rgb) - 0.5) * 2.0);

#if 0
	vec2 texofs = 1.0 / textureSize(uSamplerNormalAmbOcc, 0);
	
	vec3 n_n = acquireAdjacentNormal(uSamplerNormalAmbOcc, fTex0, vec2(0, -texofs.y), norm);
	vec3 n_s = acquireAdjacentNormal(uSamplerNormalAmbOcc, fTex0, vec2(0, texofs.y), norm);
	vec3 n_e = acquireAdjacentNormal(uSamplerNormalAmbOcc, fTex0, vec2(texofs.x, 0), norm);
	vec3 n_w = acquireAdjacentNormal(uSamplerNormalAmbOcc, fTex0, vec2(-texofs.x, 0), norm);
	vec3 n_ne = acquireAdjacentNormal(uSamplerNormalAmbOcc, fTex0, vec2(texofs.x, -texofs.y), norm);
	vec3 n_nw = acquireAdjacentNormal(uSamplerNormalAmbOcc, fTex0, vec2(-texofs.x, -texofs.y), norm);
	vec3 n_se = acquireAdjacentNormal(uSamplerNormalAmbOcc, fTex0, vec2(texofs.x, texofs.y), norm);
	vec3 n_sw = acquireAdjacentNormal(uSamplerNormalAmbOcc, fTex0, vec2(-texofs.x, texofs.y), norm);

	float d_n_s = -clamp(dot(n_n, n_s), -1, 0);
	float d_e_w = -clamp(dot(n_e, n_w), -1, 0);
	float d_ne_sw = -clamp(dot(n_ne, n_sw), -1, 0);
	float d_nw_se = -clamp(dot(n_nw, n_se), -1, 0);
	float aot = 1.0 - (d_n_s * d_e_w * d_ne_sw * d_nw_se);
#endif
	
	vec3 sunlight = normalize(-uSunDirection);
	float NdotL = max(dot(norm, sunlight), 0.0);
	
	vec4 texEmissiveRoughness = texture(uSamplerEmissionRoughness, fTex0);
	vec3 texPositionDepth = texture(uSamplerPosDepth, fTex0).rgb;
	
	vec3 lights = texture(uSamplerLights, fTex0).rgb * texDiffuseMetalness.rgb;
	
	vec3 diffuse = texDiffuseMetalness.rgb * uSunColor;
	vec3 ambient = texDiffuseMetalness.rgb * uAmbientColor;
	vec3 emissive = texEmissiveRoughness.rgb;
	float metalness = texDiffuseMetalness.a;
	float roughness = texEmissiveRoughness.a;
	
#if 0
	diffuse *= aot;
	ambient *= aot;
#endif
	
	vec3 view = normalize(texPositionDepth - uEyePosition);
	vec3 refl = normalize(-reflect(-sunlight, norm));
	float spec = pow(max(dot(view, refl), 0.0), roughness * 16.0);
	vec3 specular = spec * mix(uSunColor, texDiffuseMetalness.rgb, metalness) * 0.4;
	
	vec4 shadow_pos = uMatrixS * vec4(texPositionDepth.xyz, 1.0);
	vec3 shadow_coords = (shadow_pos.xyz / shadow_pos.w) * 0.5 + 0.5;
	
	float texShadow = texture(uSamplerShadow, shadow_coords.xy).r;
	float bias = clamp(0.005 * tan(acos(NdotL)), 0.0, 0.01);  
	
	float shade = (shadow_coords.z > (texShadow + bias)) ? 0.0 : 1.0;
	
	oColor = vec4(mix(ambient, diffuse, NdotL * shade) + (specular * shade) + emissive + lights, 1);
}
