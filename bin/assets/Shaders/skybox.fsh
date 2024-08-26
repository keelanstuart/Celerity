uniform vec4 uColorDiffuse;
uniform vec4 uColorEmissive;
uniform sampler2D uSamplerDiffuse;
uniform sampler2D uSamplerEmissive;
uniform sampler2D uSamplerSurfaceDesc;
uniform float uAlphaPass;

in vec4 fPosDepth;
in vec2 fTex0;

layout (location=0) out vec4 oDefDiffuseMetalness;
layout (location=3) out vec4 oDefEmissiveRoughness;

void main()
{
	vec4 diff = texture(uSamplerDiffuse, fTex0) * uColorDiffuse;
	diff.rgb *= diff.a;

	vec4 emis = texture(uSamplerEmissive, fTex0) * uColorEmissive; 
	emis.rgb *= emis.a;

	// encode metalness as diffuse.a
	oDefDiffuseMetalness = vec4(diff.rgb, 0);

	// encode roughness as emissive.a
	oDefEmissiveRoughness = vec4(emis.rgb, 0);
}
