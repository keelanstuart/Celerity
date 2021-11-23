uniform sampler2D uSamplerDiffuse;
uniform sampler2D uSamplerNormal;
uniform sampler2D uSamplerEmissive;
uniform float uAlphaThreshold;

in vec4 fPosDepth;
in vec3 fT, fB, fN;
in vec2 fTex0;
in vec4 fColor0;

layout (location=0) out vec4 oDefColor;		// deferred color
layout (location=1) out vec4 oDefNormal;
layout (location=2) out vec4 oDefPosDepth;
layout (location=3) out vec4 oDefEmis;

void main()
{
	vec4 diff = texture(uSamplerDiffuse, fTex0);
	diff *= fColor0;
	if (diff.a < uAlphaThreshold)
		discard;

	oDefPosDepth = fPosDepth;
	oDefColor = diff;

	// construct the normal transform
	mat3 TBN = mat3(fT, fB, fN);
	vec3 texN = texture(uSamplerNormal, fTex0).rgb * 2.0 - 1.0;
	vec3 N = normalize(TBN * texN);
	oDefNormal = vec4(N.rgb, 1.0);

	vec3 emis = texture(uSamplerEmissive, fTex0).rgb * fColor0.rgb * fColor0.a; 
	oDefEmis = vec4(emis, 0);
}
