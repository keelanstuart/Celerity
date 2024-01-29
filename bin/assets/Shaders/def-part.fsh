uniform sampler2D uSamplerDiffuse;
uniform float uAlphaPass;

in vec4 fPosDepth;
in vec2 fTex0;
in vec4 fColor0;

layout (location=0) out vec4 oDefDiffuseMetalness;

void main()
{
	vec4 texDiffuse = texture(uSamplerDiffuse, fTex0);
	texDiffuse *= fColor0;

	oDefDiffuseMetalness = texDiffuse;
}
