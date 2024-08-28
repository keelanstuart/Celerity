uniform mat4 uMatrixM;
uniform mat4 uMatrixMVP;

layout (location=0) in vec3 vPos;
layout (location=1) in vec3 vNorm;
layout (location=2) in vec3 vTan;
layout (location=3) in vec3 vBinorm;
layout (location=4) in vec2 vTex0;

out vec4 fPosDepth;

void main()
{
	vec4 wpos = vec4(vPos, 1.0);
	vec4 zpos = uMatrixMVP * wpos;
	gl_Position = zpos;
	fPosDepth = vec4(vec4(uMatrixM * wpos).xyz, zpos.w);
}
