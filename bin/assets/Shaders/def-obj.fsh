uniform sampler2D uSamplerDiffuse;
uniform sampler2D uSamplerNormal;
uniform sampler2D uSamplerEmissive;

in vec3 fPosDepth;
in mat3 fTBN;
in vec2 fTex0;
in vec4 fColor0;

layout (location=0) out vec4 oDefColor;		// deferred color
layout (location=1) out vec3 oDefNormal;
layout (location=2) out vec3 oDefPosDepth;
layout (location=3) out vec3 oDefEmis;

void main()
{
	vec4 diff = texture(uSamplerDiffuse, fTex0);
	if (diff.a == 0)
		discard;

	diff.rgb *= fColor0.rgb;

	vec3 texN = texture(uSamplerNormal, fTex0).rgb * 2.0 - 1.0;
	vec3 N = normalize(fTBN * texN);

	oDefEmis = texture(uSamplerEmissive, fTex0).rgb * fColor0.rgb * fColor0.a;

	oDefColor = diff;
	oDefNormal = N.rgb;
	oDefPosDepth = fPosDepth;
}
