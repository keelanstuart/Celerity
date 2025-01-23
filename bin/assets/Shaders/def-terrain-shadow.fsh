uniform sampler2D uSamplerDiffuse;
uniform float uAlphaPass;

in vec2 fTex0;

layout (location=0) out float oFragDepth;

void main()
{
	vec4 texDiffuse = texture(uSamplerDiffuse, fTex0);
	if (texDiffuse.a <= uAlphaPass)
		discard;

	oFragDepth = gl_FragCoord.z;
}
