uniform mat4 uMatrixM;
uniform mat4 uMatrixMVP;
uniform mat4 uMatrixN;
uniform vec4 uColorDiffuse;
#if defined(SKIN)
uniform mat4 uMatrixBones[64];
#endif

layout (location=0) in vec3 vPos;
layout (location=1) in vec3 vNorm;
layout (location=2) in vec3 vTan;
layout (location=3) in vec3 vBinorm;
layout (location=4) in vec2 vTex0;
#if defined(SKIN)
layout (location=5) in ivec3 vBoneIdx;
layout (location=6) in vec3 vBoneWt;
#endif

out vec4 fPosDepth;
out vec3 fT, fB, fN;
out vec2 fTex0;
out vec4 fColor0;

void main()
{
	vec4 wpos = vec4(vPos, 1.0);

#if defined(SKIN)
    mat4 matBones = 
        uMatrixBones[vBoneIdx.x] * vBoneWt.x +
        uMatrixBones[vBoneIdx.y] * vBoneWt.y +
        uMatrixBones[vBoneIdx.z] * vBoneWt.z;

	vec4 zpos = uMatrixMVP * (matBones * wpos);
#else
	vec4 zpos = uMatrixMVP * wpos;
#endif

	gl_Position = zpos;

	fPosDepth = vec4(vec4(uMatrixM * wpos).xyz, zpos.w);
	fColor0 = uColorDiffuse;

#if defined(SKIN)
	mat4 matBonesN = uMatrixN * boneTransform;
	fN = (matBonesN * normalize(vec4(vNorm, 0))).xyz;
	fT = (matBonesN * normalize(vec4(vTan, 0))).xyz;
	fB = (matBonesN * normalize(vec4(vBinorm, 0))).xyz;
#else
	fN = (uMatrixN * normalize(vec4(vNorm, 0))).xyz;
	fT = (uMatrixN * normalize(vec4(vTan, 0))).xyz;
	fB = (uMatrixN * normalize(vec4(vBinorm, 0))).xyz;
#endif

	fTex0 = vec2(vTex0.x, vTex0.y);
}
