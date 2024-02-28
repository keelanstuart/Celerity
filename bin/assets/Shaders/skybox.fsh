uniform vec4 uColorDiffuse;
uniform sampler2D uSamplerDiffuse;
uniform sampler2D uSamplerNormal;
uniform sampler2D uSamplerEmissive;
uniform sampler2D uSamplerSurfaceDesc;
uniform vec3 uSunDirection;
uniform float uAlphaPass;

in vec4 fPosDepth;
in vec2 fTex0;

layout (location=0) out vec4 oDefDiffuseMetalness;

void main()
{
/*
	vec3 to_pix = normalize(fPosDepth.xyz);

	float u = dot(vec3(1, 0, 0), to_pix);
	float v = dot(vec3(0, 1, 0), to_pix);
	vec2 uv = vec2(u, v) * 0.5 + 0.5;
*/

	vec4 tex = texture(uSamplerDiffuse, fTex0) * uColorDiffuse;

	oDefDiffuseMetalness = tex;
}
