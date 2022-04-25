uniform sampler2D uSamplerNormalAmbOcc;
uniform sampler2D uSamplerPosDepth;
uniform sampler2D uSamplerAttenuation;
uniform vec3 uLightColor;
uniform vec3 uLightPos;
uniform float uLightRadius;
uniform vec2 uScreenSize;

layout (location=0) out vec3 oColor;

void main()
{
	vec2 uv = (gl_FragCoord.xy / uScreenSize);

	vec3 ppos = texture(uSamplerPosDepth, uv).xyz;
	vec3 tolight = uLightPos - ppos;
	float dist = length(tolight);
	if (dist > uLightRadius)
		discard;

	vec3 pnorm = normalize(texture(uSamplerNormalAmbOcc, uv).xyz * 2.0 - 1.0);

	vec2 attenuv = vec2(1.0 - (dist / uLightRadius), 0);
	vec3 lightstrength = texture(uSamplerAttenuation, attenuv).xyz;

	float ndl = clamp(dot(pnorm, normalize(tolight)), 0, 1);

	oColor = vec3(uLightColor * ndl) * lightstrength;
}
