uniform mat4 uMatrixMVP;
uniform mat4 uMatrixN;

layout (location=0) in vec3 vPos;
layout (location=1) in vec3 vNorm;
layout (location=2) in vec3 vTan;
layout (location=3) in vec3 vBinorm;
layout (location=4) in vec2 vTex0;

out mat3 fTBN;
out vec4 fColor;
out vec2 fTex0;

void main()
{
	gl_Position = uMatrixMVP * vec4(vPos, 1.0);
	vec4 N = uMatrixN * vec4(vNorm, 0.0);
	vec4 T = uMatrixN * vec4(vTan, 0.0);
	vec4 B = uMatrixN * vec4(vBinorm, 0.0);
	fTBN = mat3(T.xyz, B.xyz, N.xyz);
	fTex0 = vTex0;
	fColor = vec4(1.0, 1.0, 1.0, 1.0);
}
