uniform sampler2D uSamplerSceneMip0;
uniform sampler2D uSamplerSceneMip1;
uniform sampler2D uSamplerSceneMip2;
uniform sampler2D uSamplerSceneMip3;
uniform sampler2D uSamplerPosDepth;
uniform vec2 uDepthRangeMinMax;
uniform vec2 uFocusRange;
uniform float uFocusFalloff;

in vec2 fTex0;

layout (location=0) out vec4 oColor;

void main()
{
	vec4 posd = texture(uSamplerPosDepth, fTex0);

	vec4 smip0 = texture(uSamplerSceneMip0, fTex0);
	vec4 smip1 = texture(uSamplerSceneMip1, fTex0);
	vec4 smip2 = texture(uSamplerSceneMip2, fTex0);
	vec4 smip3 = texture(uSamplerSceneMip3, fTex0);

	//float fblur = smoothstep(mindist, maxdist, abs(posd.a - uFocusDist));
	oColor = vec4((smip0.rgb + smip1.rgb + (smip2.rgb * smip2.rgb) + (smip3.rgb * smip3.rgb * smip3.rgb)) * 0.5, smip0.a);
}
