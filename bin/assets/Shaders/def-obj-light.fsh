uniform sampler2D uSamplerDiffuse;
uniform sampler2D uSamplerNormal;
uniform sampler2D uSamplerEmissive;
uniform sampler2D uSamplerSurfaceDesc;
uniform vec3 uSunDirection;
uniform float uAlphaPass;
uniform vec3 uLightColor;

in vec4 fPosDepth;
in vec3 fT, fB, fN;
in vec2 fTex0;
in vec4 fColor0;

layout (location=0) out vec4 oDefDiffuseMetalness;
layout (location=1) out vec4 oDefNormalAmbOcc;
layout (location=2) out vec4 oDefPosDepth;
layout (location=3) out vec4 oDefEmissiveRoughness;

void main()
{
	// encode metalness as diffuse.a
	oDefDiffuseMetalness = vec4(0, 0, 0, 0);

	// encode ambient occlusion as normal.a
	oDefNormalAmbOcc = vec4(uSunDirection, 0);

	oDefPosDepth = fPosDepth;

	// encode roughness as emissive.a
	oDefEmissiveRoughness = vec4(uLightColor, 0);
}
