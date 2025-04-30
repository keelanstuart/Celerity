uniform mat4 uMatrixM;
uniform mat4 uMatrixMVP;
uniform mat4 uMatrixN;
uniform vec4 uColorDiffuse;
#if defined(SKIN)
uniform mat4 uMatrixBones[128];
#endif

layout (location=0) in vec3 vPos;
layout (location=1) in vec3 vNorm;
layout (location=2) in vec3 vTan;
layout (location=3) in vec3 vBinorm;
layout (location=4) in vec2 vTex0;
#if defined(SKIN)
layout (location=5) in uvec4 vBoneIdx;
layout (location=6) in vec4 vBoneWt;
#endif

out vec4 fPosDepth;
out vec3 fT, fB, fN;
out vec2 fTex0;
out vec4 fColor0;

void main()
{
	vec4 wpos = vec4(vPos, 1.0);
    
#if defined(SKIN)
	vec4 skinnedPos = vec4(0.0);
	vec3 skinnedNorm = vec3(0.0);
	vec3 skinnedTan = vec3(0.0);
	vec3 skinnedBinorm = vec3(0.0);
	mat4 boneMatrix;

	for (int i = 0; i < 4; ++i)
	{
		boneMatrix = uMatrixBones[vBoneIdx[i]];
		skinnedPos += boneMatrix * wpos * vBoneWt[i];
		skinnedNorm += mat3(boneMatrix) * vNorm * vBoneWt[i];
		skinnedTan += mat3(boneMatrix) * vTan * vBoneWt[i];
		skinnedBinorm += mat3(boneMatrix) * vBinorm * vBoneWt[i];
	}

	vec4 zpos = uMatrixMVP * skinnedPos;
#else
	vec4 zpos = uMatrixMVP * wpos;
#endif

	gl_Position = zpos;
	fColor0 = uColorDiffuse;

#if defined(SKIN)
	fPosDepth = vec4(vec4(uMatrixM * skinnedPos).xyz, 1);
	fN = normalize(uMatrixN * vec4(skinnedNorm, 0.0)).xyz;
	fT = normalize(uMatrixN * vec4(skinnedTan, 0.0)).xyz;
	fB = normalize(uMatrixN * vec4(skinnedBinorm, 0.0)).xyz;
#else
	fPosDepth = vec4(vec4(uMatrixM * wpos).xyz, 1);
	fN = normalize(uMatrixN * vec4(vNorm, 0.0)).xyz;
	fT = normalize(uMatrixN * vec4(vTan, 0.0)).xyz;
	fB = normalize(uMatrixN * vec4(vBinorm, 0.0)).xyz;
#endif

	fTex0 = vTex0;
}
