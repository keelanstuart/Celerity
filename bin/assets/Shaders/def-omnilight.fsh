uniform sampler2D uSamplerNormal;
uniform sampler2D uSamplerPosDepth;
uniform vec3 uLightColor;
uniform vec3 uLightPos;
uniform float uLightRadius;
uniform vec2 uScreenSize;

layout (location=0) out vec4 oColor;

void main()
{
	vec2 uv = gl_FragCoord.xy / uScreenSize;

	vec3 ppos = texture(uSamplerPosDepth, uv).xyz;
	vec3 tolight = ppos - uLightPos;
	float dist = length(tolight);
	if (dist > uLightRadius)
		discard;

	vec3 pnorm = texture(uSamplerNormal, uv).xyz;

	float lightstrength = 1 - dist / uLightRadius;

	float ndl = clamp(dot(pnorm, normalize(tolight)), 0, 1);

	oColor = vec4(uLightColor * ndl * lightstrength, 1);
}
