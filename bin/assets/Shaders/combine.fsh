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

void main()
{
	vec4 texNormalAmbOcc = texture(uSamplerNormalAmbOcc, fTex0);
	
	if (texNormalAmbOcc.rgb == vec3(0, 0, 0))
		discard;
	
	vec3 norm = normalize((normalize(texNormalAmbOcc.rgb) - 0.5) * 2.0);
	
	vec3 sunlight = normalize(-uSunDirection);
	float NdotL = max(dot(norm, sunlight), 0.0);
	
	vec4 texDiffuseMetalness = texture(uSamplerDiffuseMetalness, fTex0);
	vec4 texEmissiveRoughness = texture(uSamplerEmissionRoughness, fTex0);
	vec3 texPositionDepth = texture(uSamplerPosDepth, fTex0).rgb;
	
	vec3 lights = texture(uSamplerLights, fTex0).rgb * texDiffuseMetalness.rgb;
	
	vec3 diffuse = texDiffuseMetalness.rgb * uSunColor;
	vec3 ambient = texDiffuseMetalness.rgb * uAmbientColor;
	vec3 emissive = texEmissiveRoughness.rgb;
	float metalness = texDiffuseMetalness.a;
	float roughness = texEmissiveRoughness.a;
	
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
