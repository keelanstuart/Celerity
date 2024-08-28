uniform vec3 uLightColor;

in vec4 fPosDepth;

layout (location=0) out vec4 oDefDiffuseMetalness;
layout (location=1) out vec4 oDefNormalAmbOcc;
layout (location=2) out vec4 oDefPosDepth;
layout (location=3) out vec4 oDefEmissiveRoughness;

void main()
{
	oDefDiffuseMetalness = vec4(1,1,1,1);
	oDefNormalAmbOcc = vec4(1, 1, 1, 0);
	oDefPosDepth = fPosDepth;
	oDefEmissiveRoughness = vec4(uLightColor, 0);
}
