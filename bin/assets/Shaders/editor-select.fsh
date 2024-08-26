uniform sampler2D uSamplerDiffuse;
uniform sampler2D uSamplerNormal;
uniform vec3 uEyeDirection;
uniform float uAlphaPass;

in vec4 fPosDepth;
in vec3 fT, fB, fN;
in vec2 fTex0;
in vec4 fColor0;

layout (location=0) out vec2 oAux;

void main()
{
	vec4 diff = texture(uSamplerDiffuse, fTex0);
	if (diff.a <= uAlphaPass)
		discard;

	oAux = vec2(1, fPosDepth.a);
}
