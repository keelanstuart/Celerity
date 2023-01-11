uniform sampler2D uSamplerDiffuseMetalness;
uniform sampler2D uSamplerNormalAmbOcc;
uniform sampler2D uSamplerPosDepth;
uniform sampler2D uSamplerEmissionRoughness;
uniform sampler2D uSamplerLights;
uniform sampler2D uSamplerShadow;
uniform sampler2D uSamplerAuxiliary;
uniform vec3 uSunDirection;
uniform vec3 uSunColor;
uniform vec3 uAmbientColor;
uniform mat4 uMatrixS;
uniform float uElapsedTime;

in vec2 fTex0;

layout (location=0) out vec4 oColor;

void main()
{
	vec3 norm = normalize(texture(uSamplerNormalAmbOcc, fTex0).rgb * 2.0 - 1.0);
	if (norm == vec3(0, 0, 0))
		discard;

	// read position/depth first
	vec4 posd = texture(uSamplerPosDepth, fTex0);

	vec4 diff = texture(uSamplerDiffuseMetalness, fTex0);
	vec3 emis = texture(uSamplerEmissionRoughness, fTex0).rgb;
	vec3 lights = texture(uSamplerLights, fTex0).rgb;

	float ndotl = -dot(norm, uSunDirection);

	vec4 spos = uMatrixS * vec4(posd.xyz, 1.0);
	vec3 scoords = spos.xyz / spos.w;
	scoords = scoords * 0.5 + 0.5;
	float stex = texture(uSamplerShadow, scoords.xy).r;
	float bias = max(0.007 * (1.0 - ndotl), 0.0007);  
	float light = (scoords.z > (stex + bias)) ? 0.0 : 1.0;
	
	ivec2 aux_tex_size = textureSize(uSamplerAuxiliary, 0);
	vec2 aux_texel_inc;
	aux_texel_inc.x = 1.0 / float(aux_tex_size.x) * 4.0;
	aux_texel_inc.y = 1.0 / float(aux_tex_size.y) * 4.0;

	vec4 aux[9];
	aux[0] = texture(uSamplerAuxiliary, vec2(fTex0.x - aux_texel_inc.x,	fTex0.y - aux_texel_inc.y));
	aux[1] = texture(uSamplerAuxiliary, vec2(fTex0.x,					fTex0.y - aux_texel_inc.y));
	aux[2] = texture(uSamplerAuxiliary, vec2(fTex0.x + aux_texel_inc.x,	fTex0.y - aux_texel_inc.y));
	aux[3] = texture(uSamplerAuxiliary, vec2(fTex0.x - aux_texel_inc.x,	fTex0.y));
	aux[4] = texture(uSamplerAuxiliary, vec2(fTex0.x,					fTex0.y));
	aux[5] = texture(uSamplerAuxiliary, vec2(fTex0.x + aux_texel_inc.x,	fTex0.y));
	aux[6] = texture(uSamplerAuxiliary, vec2(fTex0.x - aux_texel_inc.x,	fTex0.y + aux_texel_inc.y));
	aux[7] = texture(uSamplerAuxiliary, vec2(fTex0.x,					fTex0.y + aux_texel_inc.y));
	aux[8] = texture(uSamplerAuxiliary, vec2(fTex0.x + aux_texel_inc.x,	fTex0.y + aux_texel_inc.y));

	float glow = 0;
	for (int i = 0; i < 9; i++)
		glow += aux[i].r;
	glow /= 9;
	//glow *= sin(uElapsedTime / 2.0);

	vec3 color = (diff.rgb * uAmbientColor) + (diff.rgb * lights) + emis + (diff.rgb * uSunColor * light * clamp(ndotl, 0, 1)) + ((1.0 - aux[4].r) * sin(glow * 1.5707) * 2.0);
	oColor = vec4(color, 1.0);
}
