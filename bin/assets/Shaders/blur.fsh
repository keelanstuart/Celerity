uniform sampler2D uSamplerUpRes;
uniform float uBlurScale;

in vec2 fTex0;

layout (location=0) out vec4 oColor;

void main()
{
	oColor = texture(uSamplerUpRes, vec2(fTex0.x * uBlurScale, fTex0.y * uBlurScale));
}
