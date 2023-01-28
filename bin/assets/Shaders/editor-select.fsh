uniform sampler2D uSamplerDiffuse;
uniform sampler2D uSamplerNormal;
uniform vec3 uEyeDirection;
uniform float uAlphaPass;

out vec4 fPosDepth;
out vec3 fT, fB, fN;
out vec2 fTex0;
out vec4 fColor0;

layout (location=0) out vec3 oAux;

void main()
{
	vec4 diff = texture(uSamplerDiffuse, fTex0);
	if (diff.a <= uAlphaPass)
		discard;

	mat3 TBN = mat3(fT, fB, fN);
	vec3 texN = normalize((texture(uSamplerNormal, fTex0).rgb + 1.0) * 0.5);
	vec3 N = normalize(TBN * texN);
	float ndote = abs(dot(N, uEyeDirection));
	if (ndote > 0.1)
		discard;

	oAux = vec3(1, 0, 0);
}
