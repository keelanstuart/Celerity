uniform vec3 uLightColor;

//layout (location=0) out vec4 oDefDiffuseMetalness;
layout (location=3) out vec4 oDefEmissionRoughness;

void main()
{
	//oDefDiffuseMetalness = vec4(1,1,1,0);//uLightColor.rgb, 0);
	oDefEmissionRoughness = vec4(uLightColor.rgb * 10.0, 0);
}
