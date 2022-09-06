uniform float uAlphaPass;

in vec4 fColor0;

layout (location=0) out vec4 fragment;

void main()
{
	if (fColor0.a <= uAlphaPass)
		discard;

	fragment = fColor0;
}
