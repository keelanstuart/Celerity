uniform mat4 uMatrixM;
uniform mat4 uMatrixS;

layout (location=0) in vec3 vPos;
layout (location=1) in vec3 vNorm;
layout (location=2) in vec3 vTan;
layout (location=3) in vec3 vBinorm;
layout (location=4) in vec2 vTex0;

void main()
{
	gl_Position = uMatrixS * uMatrixM * vec4(vPos, 1.0);
}
