uniform sampler2D uSamplerDiffuse;
uniform sampler2D uSamplerNormal;
uniform sampler2D uSamplerEmissive;
uniform sampler2D uSamplerSurfaceDesc;
uniform float uAlphaPass;

in vec4 fPosDepth;
in vec3 fT, fB, fN;
in vec2 fTex0;
in vec4 fColor0;

layout (location=0) out vec4 oDefDiffuseMetalness;
layout (location=1) out vec4 oDefNormalAmbOcc;
layout (location=2) out vec4 oDefPosDepth;
layout (location=3) out vec4 oDefEmissiveRoughness;

void main()
{
	vec4 fonttex = vec4(1, 1, 1, texture(uSamplerDiffuse, fTex0).r);
	fonttex *= fColor0;
	if (fonttex.a == 0)
		discard;

	// construct the normal transform
	mat3 TBN = mat3(normalize(fT), normalize(fB), normalize(fN));
	vec3 texN = normalize((texture(uSamplerNormal, fTex0).rgb - 0.5) * 2.0);
	vec3 N = normalize(TBN * texN);

	// surface description texture (UE4 style)
	//  .r = ambient occlusion
	//	.g = roughness
	//	.b = metalness
	vec4 texSurfaceDesc = texture(uSamplerSurfaceDesc, fTex0);

	vec4 texEmissive = texture(uSamplerEmissive, fTex0); 

	// encode metalness as diffuse.a
	oDefDiffuseMetalness = fonttex;

	// encode ambient occlusion as normal.a
	oDefNormalAmbOcc = vec4(N * 0.5 + 0.5, texSurfaceDesc.r);

	oDefPosDepth = fPosDepth;

	// encode roughness as emissive.a
	oDefEmissiveRoughness = vec4(texEmissive.rgb * fonttex.a, texSurfaceDesc.g);

    // Modify the depth (e.g., apply an offset)
    float depthBias = 0.0001; // Example small bias
    gl_FragDepth = fPosDepth.z / fPosDepth.w + depthBias;

}
