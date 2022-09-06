uniform sampler2D uSamplerSceneMip0;
uniform sampler2D uSamplerSceneMip1;
uniform sampler2D uSamplerSceneMip2;
uniform sampler2D uSamplerSceneMip3;
uniform sampler2D uSamplerPosDepth;
uniform float uFocusDist;
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
	//oColor = mix((smip0 * 0.8) + (smip3 * 0.2), (smip1 * 0.8) + (smip2 * 0.2), fblur);
	oColor = (smip0 * 0.45) + (smip1 * 0.35) + (smip2 * 0.25) + (smip3 * 0.15);
}
