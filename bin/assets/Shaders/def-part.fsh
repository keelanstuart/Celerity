uniform sampler2D uSamplerDiffuse;
uniform float uAlphaPass;

in vec4 fPosDepth;
in vec2 fTex0;
in vec4 fColor0;

layout (location=0) out vec4 oDefDiffuseMetalness;
layout (location=3) out vec4 oDefEmissionRoughness;


void main()
{
	vec4 tex = texture(uSamplerDiffuse, fTex0);
	tex *= fColor0;

	oDefDiffuseMetalness = tex;
	oDefEmissionRoughness = tex;
}
