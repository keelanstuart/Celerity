uniform sampler2D uSamplerSceneMip0;
uniform sampler2D uSamplerSceneMip1;
uniform sampler2D uSamplerSceneMip2;
uniform sampler2D uSamplerSceneMip2_1;
uniform sampler2D uSamplerSceneMip2_2;
uniform sampler2D uSamplerSceneMip2_3;
uniform sampler2D uSamplerSceneMip2_4;
uniform sampler2D uSamplerSceneMip3;
uniform sampler2D uSamplerPosDepth;
uniform vec2 uDepthRangeMinMax;
uniform vec2 uFocusRange;
uniform float uFocusFalloff;

in vec2 fTex0;

layout (location=0) out vec4 oColor;

void main()
{
	vec2 uv = vec2(fTex0.x + 1, fTex0.y);

	vec4 posd = texture(uSamplerPosDepth, uv);
	vec4 smip0 = texture(uSamplerSceneMip0, uv);
	vec4 smip1 = texture(uSamplerSceneMip1, uv);
	vec4 smip2 = (texture(uSamplerSceneMip2, uv) / 5.0);
	smip2 += (texture(uSamplerSceneMip2_1, uv) / 5.0);
	smip2 += (texture(uSamplerSceneMip2_2, uv) / 5.0);
	smip2 += (texture(uSamplerSceneMip2_3, uv) / 5.0);
	smip2 += (texture(uSamplerSceneMip2_4, uv) / 5.0);
	vec4 smip3 = texture(uSamplerSceneMip3, uv);

	smip1 *= 0.5;

	smip2 *= smip2;
	smip2 *= 0.7;

	smip3 *= smip3 * smip3;
	smip3 *= 0.9;

	vec3 cc = smip0.rgb + smip1.rgb + smip2.rgb + smip3.rgb;

	oColor = vec4(cc, 1);
}
