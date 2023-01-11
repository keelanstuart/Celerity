uniform sampler2D uSamplerDiffuse;
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

	oAux = vec3(1, 0, 0);
}
