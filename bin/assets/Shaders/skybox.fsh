uniform sampler2D uSamplerDiffuse;
uniform sampler2D uSamplerNormal;
uniform sampler2D uSamplerEmissive;
uniform sampler2D uSamplerSurfaceDesc;
uniform vec3 uSunDirection;
uniform float uAlphaPass;

in vec4 fPosDepth;
in vec2 fTex0;
in vec4 fColor0;

layout (location=0) out vec4 oDefDiffuseMetalness;
layout (location=1) out vec4 oDefNormalAmbOcc;
layout (location=2) out vec4 oDefPosDepth;
layout (location=3) out vec4 oDefEmissiveRoughness;

void main()
{
	vec4 tex = texture(uSamplerDiffuse, fTex0);
	tex *= fColor0;

	oDefDiffuseMetalness = vec4(0, 0, 0, 1);

	oDefNormalAmbOcc = vec4(1, 1, 1, 0);

	oDefPosDepth = fPosDepth;

	// encode roughness as emissive.a
	oDefEmissiveRoughness = vec4(tex.rgb, 1);
}
