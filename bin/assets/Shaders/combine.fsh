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

in vec2 fTex0;

layout (location=0) out vec4 oColor;

void main()
{
	vec3 norm = normalize(texture(uSamplerNormalAmbOcc, fTex0).rgb * 2.0 - 1.0);
	if (norm == vec3(0, 0, 0))
		discard;

	// read position/depth first
	vec4 posd = texture(uSamplerPosDepth, fTex0);

	vec4 diff = texture(uSamplerDiffuseMetalness, fTex0);
	vec3 emis = texture(uSamplerEmissionRoughness, fTex0).rgb;
	vec3 lights = texture(uSamplerLights, fTex0).rgb;

	float ndotl = -dot(norm, uSunDirection);

	vec4 spos = uMatrixS * vec4(posd.xyz, 1.0);
	vec3 scoords = spos.xyz / spos.w;
	scoords = scoords * 0.5 + 0.5;
	float stex = texture(uSamplerShadow, scoords.xy).r;
	float bias = max(0.007 * (1.0 - ndotl), 0.0007);  
	float light = (scoords.z > (stex + bias)) ? 0.0 : 1.0;
	
	vec3 color = (diff.rgb * uAmbientColor) + (diff.rgb * lights) + emis + (diff.rgb * uSunColor * light * clamp(ndotl, 0, 1));
	oColor = vec4(color, 1.0);
}
