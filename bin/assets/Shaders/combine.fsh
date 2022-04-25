uniform sampler2D uSamplerDiffuseMetalness;
uniform sampler2D uSamplerNormalAmbOcc;
uniform sampler2D uSamplerPosDepth;
uniform sampler2D uSamplerEmissionRoughness;
uniform sampler2D uSamplerLights;
uniform vec3 uSunDirection;
uniform vec3 uSunColor;
uniform vec3 uAmbientColor;

in vec2 fTex0;

layout (location=0) out vec4 oColor;

void main()
{
	vec3 norm = normalize(texture(uSamplerNormalAmbOcc, fTex0).rgb * 2.0 - 1.0);
	if (norm == vec3(0, 0, 0))
		discard;

	vec4 diff = texture(uSamplerDiffuseMetalness, fTex0);
	vec4 posd = texture(uSamplerPosDepth, fTex0);
	vec3 emis = texture(uSamplerEmissionRoughness, fTex0).rgb;
	vec3 lights = texture(uSamplerLights, fTex0).rgb;

	vec3 color = (diff.rgb * uAmbientColor) + (diff.rgb * lights) + emis + (diff.rgb * uSunColor * clamp(dot(norm, uSunDirection), 0, 1));
	oColor = vec4((norm == vec3(0, 0, 0)) ? vec3(1, 0, 1) : color, 1);
}
