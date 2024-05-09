uniform sampler2D uSamplerDiffuse;
uniform vec4 uColorDiffuse;

in vec2 fTex0;

layout (location = 0) out vec4 oColor;

void main()
{
    oColor = uColorDiffuse * vec4(1, 1, 1, texture(uSamplerDiffuse, fTex0.st).r);
}
