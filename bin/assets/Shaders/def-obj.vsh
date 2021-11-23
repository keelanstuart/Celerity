uniform mat4 uMatrixM;
uniform mat4 uMatrixMVP;
uniform mat4 uMatrixN;
uniform vec4 uColorDiffuse;

layout (location=0) in vec3 vPos;
layout (location=1) in vec3 vNorm;
layout (location=2) in vec3 vTan;
layout (location=3) in vec3 vBinorm;
layout (location=4) in vec2 vTex0;

out vec4 fPosDepth;
out vec3 fT, fB, fN;
out vec2 fTex0;
out vec4 fColor0;

void main()
{
	vec4 wpos = vec4(vPos, 1.0);
	fPosDepth = vec4(uMatrixM * wpos);
	gl_Position = uMatrixMVP * wpos;

	fN = normalize(uMatrixN * vec4(vNorm, 1)).xyz;
	fT = normalize(uMatrixN * vec4(vTan, 1)).xyz;
	fB = normalize(uMatrixN * vec4(vBinorm, 1)).xyz;
	//fTBN = mat3(T, B, N);

	fTex0 = vTex0;
	fColor0 = uColorDiffuse;
}
