uniform sampler2D uSamplerDiffuse;
uniform sampler2D uSamplerNormal;
uniform sampler2D uSamplerEmissive;
uniform sampler2D uSamplerSurfaceDesc;		// ao / roughness / metalness (UE4 style)
uniform float uAlphaPass;

uniform vec4 uColorDiffuse;

in vec4 fPosDepth;
in vec3 fT, fB, fN;
in vec2 fTex0;
in vec4 fColor0;

layout (location=0) out vec4 oDefDiffuseMetalness;		// albedo color
layout (location=1) out vec4 oDefNormalAmbOcc;
layout (location=2) out vec4 oDefPosDepth;
layout (location=3) out vec4 oDefEmissionRoughness;

void main()
{
	vec4 diff = texture(uSamplerDiffuse, fTex0) * uColorDiffuse;
	diff *= fColor0;
	if (diff.a <= uAlphaPass)
		discard;

	oDefPosDepth = fPosDepth;

	// construct the normal transform
	mat3 TBN = mat3(fT, fB, fN);
	vec3 texN = normalize(texture(uSamplerNormal, fTex0).rgb) * 2.0 - 1.0;
	vec3 N = normalize(TBN * texN);
	oDefNormalAmbOcc.rgb = (N.rgb + 1.0) / 2.0;

	oDefDiffuseMetalness.rgb = diff.rgb;

	vec4 surfdesc = texture(uSamplerSurfaceDesc, fTex0);
	vec4 emis = texture(uSamplerEmissive, fTex0); 

	oDefNormalAmbOcc.a = surfdesc.r;												// encode ambient occlusion as normal.a
	oDefEmissionRoughness = vec4((emis.rgb * fColor0.rgb) * fColor0.a, surfdesc.g);	// encode roughness as emissive.a
	oDefDiffuseMetalness.a = surfdesc.b;											// encode metalness as diffuse.a
}
