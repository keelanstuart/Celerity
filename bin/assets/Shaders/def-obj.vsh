uniform mat4 uMatrixM;
uniform mat4 uMatrixMVP;
uniform mat4 uMatrixN;
uniform vec4 uColorDiffuse;

layout (location=0) in vec3 vPos;
layout (location=1) in vec3 vNorm;
layout (location=2) in vec3 vTan;
layout (location=3) in vec3 vBinorm;
layout (location=4) in vec2 vTex0;

out vec3 fPosDepth;
out mat3 fTBN;
out vec2 fTex0;
out vec4 fColor0;

void main()
{
	vec4 wpos = vec4(vPos, 1.0);
	fPosDepth = vec4(uMatrixM * wpos).xyz;
	gl_Position = uMatrixMVP * wpos;

	vec3 N = normalize(uMatrixN * vec4(vNorm, 0)).xyz;
	vec3 T = normalize(uMatrixN * vec4(vTan, 0)).xyz;
	vec3 B = normalize(uMatrixN * vec4(vBinorm, 0)).xyz;
	fTBN = mat3(T, B, N);

	fTex0 = vTex0;
	fColor0 = uColorDiffuse;
}
