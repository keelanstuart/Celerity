uniform float uAlphaPass;

in vec4 fPosDepth;
in vec3 fT, fB, fN;
in vec2 fTex0;
in vec4 fColor0;

layout (location=0) out vec4 oDefColor;		// albedo color
layout (location=1) out vec4 oDefNormal;
layout (location=2) out vec4 oDefPosDepth;
layout (location=3) out vec4 oDefEmis;

void main()
{
	if (fColor0.a <= uAlphaPass)
		discard;

	oDefPosDepth = fPosDepth;
	oDefColor = vec4(0, 0, 0, 0);

	// construct the normal transform
	mat3 TBN = mat3(fT, fB, fN);
	vec3 N = normalize(TBN * vec3(0, 0, 1));
	oDefNormal = vec4(N.rgb, 1.0);

	oDefNormal.a = 0;											// encode ambient occlusion as normal.a
	oDefEmis = fColor0;	// encode roughness as emissive.a
	oDefColor.a = 0;											// encode metalness as diffuse.a
}
