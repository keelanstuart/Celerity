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
	fonttex.a *= fonttex.a;
	if (fonttex.a == 0)
		discard;

    // Modify the depth (e.g., apply an offset)
    float depthBias = 0.00001; // Example small bias
    gl_FragDepth = fPosDepth.z / fPosDepth.w + depthBias;

	// surface description texture (UE4 style)
	//  .r = ambient occlusion
	//	.g = roughness
	//	.b = metalness
	vec4 texSurfaceDesc = texture(uSamplerSurfaceDesc, fTex0);

	vec4 texEmissive = texture(uSamplerEmissive, fTex0); 

	// output real alpha, enable alpha blending, and disable alpha writes
	oDefDiffuseMetalness = fonttex;

	// encode ambient occlusion as normal.a
	oDefNormalAmbOcc = vec4(0, 0, 0, texSurfaceDesc.r);

	oDefPosDepth = fPosDepth;

	// encode roughness as emissive.a
	oDefEmissiveRoughness = vec4(texEmissive.rgb * fonttex.a, texSurfaceDesc.g);
}
