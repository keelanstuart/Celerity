//uniform vec4 uColorDiffuse;

layout (location=0) in vec3 vPos;
layout (location=1) in float vSize;
layout (location=2) in float vRoll;
layout (location=3) in vec4 vColor0;

out vec3 gPos;
out float gSize;
out float gRoll;
out vec4 gColor0;

void main()
{
	gPos = vPos;
	gSize = vSize;
	gRoll = vRoll;
	gColor0 = vColor0;// * uColorDiffuse;
}
