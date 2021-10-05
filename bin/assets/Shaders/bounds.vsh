uniform mat4 uMatrixMVP;
uniform mat4 uMatrixN;

layout (location=0) in vec3 vPos;
layout (location=1) in vec3 vNorm;
layout (location=4) in vec2 vTex0;

out vec4 fNorm;
out vec4 fColor;
out vec2 fTex0;

void main()
{
	gl_Position = uMatrixMVP * vec4(vPos, 1.0);
	fNorm = uMatrixN * vec4(vNorm, 1.0);
	fTex0 = vTex0;
	fColor = vec4(1.0, 1.0, 1.0, 1.0);
}
