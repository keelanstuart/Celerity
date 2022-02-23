uniform sampler2D uSamplerDiffuse;
uniform vec4 uColorDiffuse;

in vec3 fNorm;
in vec4 fColor;
in vec2 fTex0;

layout (location=0) out vec4 fragment;

void main()
{
	fragment = texture(uSamplerDiffuse, fTex0) * uColorDiffuse;

	if (fragment.a < 0.1)
		discard;
}
