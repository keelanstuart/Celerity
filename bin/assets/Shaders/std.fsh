in mat3 fTBN;
in vec4 fColor;
in vec2 fTex0;

uniform sampler2D uSamplerDiffuse;
uniform sampler2D uSamplerNormal;
uniform vec4 uColorDiffuse;

layout (location=0) out vec4 oColor;
layout (location=1) out vec4 oNormal;

void main()
{
	vec4 diff = texture(uSamplerDiffuse, fTex0);// * uColorDiffuse;
	//if (diff.a < 0.012)	// 3 / 255
	//	discard;

	vec3 texN = texture(uSamplerNormal, fTex0).rgb;
	texN = normalize(texN * 2.0 - 1.0);
	vec3 N = normalize(fTBN * texN);

	oColor = diff;
	oNormal = vec4(N.rgb, 1.0);
}
