uniform mat4 uMatrixM;
uniform mat4 uMatrixS;
uniform sampler2D uSamplerHeight;

layout (location=0) in vec3 vPos;
layout (location=1) in vec3 vNorm;
layout (location=2) in vec3 vTan;
layout (location=3) in vec3 vBinorm;
layout (location=4) in vec2 vTex0;

out vec2 fTex0;

void main()
{
	vec4 wpos = vec4(vPos, 1.0);

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
	}
	else
	{
		ivec2 tex_size = textureSize(uSamplerHeight, 0);
		vec2 texel_inc;
		texel_inc.x = 1.0 / float(tex_size.x);
		texel_inc.y = 1.0 / float(tex_size.y);

		h_n = texture2D(uSamplerHeight, vec2(vTex0.x, vTex0.y + texel_inc.y)).r;
		h_e = texture2D(uSamplerHeight, vec2(vTex0.x + texel_inc.x, vTex0.y)).r;
		h_s = texture2D(uSamplerHeight, vec2(vTex0.x, vTex0.y - texel_inc.y)).r;
		h_w = texture2D(uSamplerHeight, vec2(vTex0.x - texel_inc.x, vTex0.y)).r;

		wpos.z = texture2D(uSamplerHeight, vec2(vTex0.x, vTex0.y)).r;
	}

	gl_Position = uMatrixS * uMatrixM * wpos;
	fTex0 = vTex0;
}
