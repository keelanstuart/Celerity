uniform sampler2D uSamplerDiffuseMetalness;
uniform sampler2D uSamplerNormalAmbOcc;
uniform sampler2D uSamplerPosDepth;
uniform sampler2D uSamplerEmissionRoughness;
uniform sampler2D uSamplerAttenuation;
uniform vec3 uLightColor;
uniform vec3 uLightPos;
uniform float uLightRadius;
uniform vec2 uScreenSize;
uniform vec3 uEyePosition;
uniform vec3 uEyeDirection;
uniform float uAttenuation;

layout (location=0) out vec3 oColor;

void main()
{
	vec2 uv = (gl_FragCoord.xy / uScreenSize);

	vec4 posdepth = texture(uSamplerPosDepth, uv);

	vec3 tolight = uLightPos - posdepth.xyz;
	float dist = length(tolight);
	if (dist > uLightRadius)
		discard;

	vec4 texDiffuseMetalness = texture(uSamplerDiffuseMetalness, uv);
	float metalness = texDiffuseMetalness.a;
	float roughness = texture(uSamplerEmissionRoughness, uv).a;

	vec3 pnorm = texture(uSamplerNormalAmbOcc, uv).xyz * 2.0 - 1.0;

	vec2 attenuv = vec2(1.0 - (dist / uLightRadius), 0);
	vec3 lightstrength = texture(uSamplerAttenuation, attenuv).xyz;
	lightstrength *= uAttenuation;

	float ndl = clamp(dot(pnorm, normalize(tolight)), 0, 1);

	vec3 view = normalize(posdepth.xyz - uEyePosition);
	vec3 refl = normalize(-reflect(tolight, pnorm));
	float spec = pow(max(dot(view, refl), 0.0), roughness * 16.0);
    vec3 specular = spec * mix(uLightColor, texDiffuseMetalness.rgb, 1 - metalness);

	oColor = (vec3(uLightColor * ndl) + specular) * lightstrength;
}
