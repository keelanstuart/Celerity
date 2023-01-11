uniform sampler2D uSamplerDiffuseMetalness;
uniform sampler2D uSamplerNormalAmbOcc;
uniform sampler2D uSamplerPosDepth;
uniform sampler2D uSamplerEmissionRoughness;
uniform sampler2D uSamplerAttenuation;
uniform vec3 uLightColor;
uniform vec3 uLightPos;
uniform vec3 uEyePosition;
uniform vec3 uEyeDirection;
uniform float uLightRadius;
uniform vec2 uScreenSize;

layout (location=0) out vec3 oColor;

#define PI 3.141592653589793
#define PI2 6.283185307179586
#define PI_HALF 1.5707963267948966
#define RECIPROCAL_PI 0.3183098861837907
#define RECIPROCAL_PI2 0.15915494309189535
#define EPSILON 1e-6
#ifndef saturate
	#define saturate(a) clamp(a, 0.0, 1.0)
#endif
#define whiteComplement(a) (1.0 - saturate(a))

float pow2(float x) { return x * x; }
vec3 pow2(vec3 x) { return x * x; }
float pow3(float x) { return x * x * x; }
float pow4(float x) { float x2 = x * x; return x2 * x2; }
float max3(vec3 v) { return max(max(v.x, v.y), v.z); }
float average(vec3 v) { return dot(v, vec3(0.333333)); }

vec3 F_Schlick(vec3 specularColor, float dotLH)
{
	float fresnel = exp2( ( -5.55473 * dotLH - 6.98316 ) * dotLH );
	return ( 1.0 - specularColor ) * fresnel + specularColor;
}

float G_GGX_SmithCorrelated( const in float alpha, const in float dotNL, const in float dotNV )
{
	float a2 = pow2( alpha );
	float gv = dotNL * sqrt( a2 + ( 1.0 - a2 ) * pow2( dotNV ) );
	float gl = dotNV * sqrt( a2 + ( 1.0 - a2 ) * pow2( dotNL ) );
	return 0.5 / max( gv + gl, EPSILON );
}

float D_GGX(float alpha, float dotNH)
{
	float a2 = pow2(alpha);
	float denom = pow2(dotNH) * (a2 - 1.0) + 1.0;
	return RECIPROCAL_PI * a2 / pow2(denom);
}

vec3 BRDF_GGX(vec3 lightDir, vec3 viewDir, vec3 normal, vec3 specularColor, float roughness)
{
	float alpha = pow2( roughness );
	vec3 halfDir = normalize( lightDir + viewDir );
	float dotNL = clamp( dot( normal, lightDir ), 0, 2 );
	float dotNV = clamp( dot( normal, viewDir ), 0, 2 );
	float dotNH = clamp( dot( normal, halfDir ), 0, 2 );
	float dotLH = clamp( dot( lightDir, halfDir ), 0, 2 );
	vec3 F = F_Schlick( specularColor, dotLH );
	float G = G_GGX_SmithCorrelated( alpha, dotNL, dotNV );
	float D = D_GGX( alpha, dotNH );
	return F * ( G * D );
}

void main()
{
	vec2 uv = (gl_FragCoord.xy / uScreenSize);

	vec3 ppos = texture(uSamplerPosDepth, uv).xyz;
	vec3 tolight = uLightPos - ppos;
	float dist = length(tolight);
	if (dist > uLightRadius)
		discard;

	vec3 pnorm = texture(uSamplerNormalAmbOcc, uv).xyz * 2.0 - 1.0;
	vec4 diffmetal = texture(uSamplerDiffuseMetalness, uv);
	float metalness = 1 - diffmetal.a;
	float roughness = texture(uSamplerEmissionRoughness, uv).a;

	vec2 attenUV = vec2(1.0 - (dist / (uLightRadius + 0.01)), 0);
	vec3 lightStrength = texture(uSamplerAttenuation, attenUV).xyz;

	//float ndl = clamp(dot(pnorm, normalize(tolight)), 0, 1);

	oColor = lightStrength * uLightColor * clamp( BRDF_GGX(tolight, uEyeDirection, pnorm, clamp(diffmetal.rgb * vec3(metalness, metalness, metalness), 0, 1), roughness), 0, 2 );
}
