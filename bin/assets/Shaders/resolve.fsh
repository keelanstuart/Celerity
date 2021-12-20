uniform sampler2D uSamplerDiffuseMetalness;
uniform sampler2D uSamplerNormalAmbOcc;
uniform sampler2D uSamplerPosDepth;
uniform sampler2D uSamplerEmissionRoughness;
uniform sampler2D uSamplerLights;

in vec2 fTex0;

layout (location=0) out vec4 oColor;

void main()
{
	vec3 norm = texture(uSamplerNormalAmbOcc, fTex0).rgb;
	if (norm == vec3(0, 0, 0))
		discard;

	vec3 sunDir = normalize(vec3(0, 0.25, 1));
	vec3 sunColor = vec3(1, 1, 0.95);
	vec3 ambientColor = vec3(0.11, 0.11, 0.1);

	vec4 diff = texture(uSamplerDiffuseMetalness, fTex0);
	vec4 posd = texture(uSamplerPosDepth, fTex0);
	vec3 emis = texture(uSamplerEmissionRoughness, fTex0).rgb;
	vec3 lights = texture(uSamplerLights, fTex0).rgb;

	vec3 color = (diff.rgb * ambientColor) + (diff.rgb * sunColor * clamp(dot(norm, sunDir), 0, 1)) + emis + (diff.rgb * lights);
	oColor = vec4(color, diff.a);
}
