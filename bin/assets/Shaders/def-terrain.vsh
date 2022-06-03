uniform mat4 uMatrixM;
uniform mat4 uMatrixMVP;
uniform mat4 uMatrixN;
uniform vec4 uColorDiffuse;
uniform sampler2D uSamplerHeight;
uniform vec2 uSamplerHeightStep;

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

	vec3 tang;
	vec3 bitang;

	float h_n, h_e, h_s, h_w;

	vec2 uv = vec2(vNorm.x, vNorm.y);

	// norm.x/y have been encoded as texture coordinate offsets (for heightmap pixel corners)
	if ((vNorm.x != 0.0) || (vNorm.y != 0.0))
	{
		float h_ne = texture2D(uSamplerHeight, vec2(vTex0.x + uv.x, vTex0.y + uv.y)).r;
		float h_se = texture2D(uSamplerHeight, vec2(vTex0.x + uv.x, vTex0.y - uv.y)).r;
		float h_nw = texture2D(uSamplerHeight, vec2(vTex0.x - uv.x, vTex0.y + uv.y)).r;
		float h_sw = texture2D(uSamplerHeight, vec2(vTex0.x - uv.x, vTex0.y - uv.y)).r;

		wpos.z = (h_ne + h_se + h_nw + h_sw) / 4.0;

		h_n = (h_ne + h_nw) / 2.0;
		h_e = (h_ne + h_se) / 2.0;
		h_s = (h_se + h_sw) / 2.0;
		h_w = (h_nw + h_sw) / 2.0;

		tang.xy = vec2(0.5, 0);
		bitang.xy = vec2(0, 0.5);
	}
	else
	{
		h_n = texture2D(uSamplerHeight, vec2(vTex0.x, vTex0.y + uSamplerHeightStep.y)).r;
		h_e = texture2D(uSamplerHeight, vec2(vTex0.x + uSamplerHeightStep.x, vTex0.y)).r;
		h_s = texture2D(uSamplerHeight, vec2(vTex0.x, vTex0.y - uSamplerHeightStep.y)).r;
		h_w = texture2D(uSamplerHeight, vec2(vTex0.x - uSamplerHeightStep.x, vTex0.y)).r;

		wpos.z = texture2D(uSamplerHeight, vec2(vTex0.x, vTex0.y)).r;

		tang.xy = vec2(1, 0);
		bitang.xy = vec2(0, 1);
	}

	tang.z = h_e - h_w;
	bitang.z = h_n - h_s;
	vec3 norm = normalize(cross(normalize(tang), normalize(bitang)));

	vec4 zpos = uMatrixMVP * wpos;
	gl_Position = zpos;
	fPosDepth = vec4(vec4(uMatrixM * wpos).xyz, zpos.w);

	fN = normalize(uMatrixN * vec4(norm, 0)).xyz;
	fT = normalize(uMatrixN * vec4(tang, 0)).xyz;
	fB = normalize(uMatrixN * vec4(bitang, 0)).xyz;

	fTex0 = vTex0;
	fColor0 = uColorDiffuse;
}
