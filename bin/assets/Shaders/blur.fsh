uniform sampler2D uSamplerUpRes;
uniform float uBlurScale;

in vec2 fTex0;

layout (location=0) out vec4 oColor;

void main()
{
	ivec2 tex_size = textureSize(uSamplerUpRes, 0);
	vec2 texel_inc;
	texel_inc.x = 1.0 / float(tex_size.x) * 4.0;
	texel_inc.y = 1.0 / float(tex_size.y) * 4.0;

	vec2 t = fTex0 * uBlurScale;
	
	vec4 b[9];
	b[0] = texture(uSamplerUpRes, vec2(t.x - texel_inc.x,	t.y - texel_inc.y)) * 0.05;
	b[1] = texture(uSamplerUpRes, vec2(t.x,					t.y - texel_inc.y)) * 0.1;
	b[2] = texture(uSamplerUpRes, vec2(t.x + texel_inc.x,	t.y - texel_inc.y)) * 0.05;
	b[3] = texture(uSamplerUpRes, vec2(t.x - texel_inc.x,	t.y)) * 0.1;
	b[4] = texture(uSamplerUpRes, vec2(t.x,					t.y)) * 0.4;
	b[5] = texture(uSamplerUpRes, vec2(t.x + texel_inc.x,	t.y)) * 0.1;
	b[6] = texture(uSamplerUpRes, vec2(t.x - texel_inc.x,	t.y + texel_inc.y)) * 0.05;
	b[7] = texture(uSamplerUpRes, vec2(t.x,					t.y + texel_inc.y)) * 0.1;
	b[8] = texture(uSamplerUpRes, vec2(t.x + texel_inc.x,	t.y + texel_inc.y)) * 0.05;

	oColor = b[0] + b[1] + b[2] + b[3] + b[4] + b[5] + b[6] + b[7] + b[8];
}
