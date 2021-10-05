uniform sampler2D uSamplerDiffuse;

in vec2 fTex0;
in vec4 fCol0;

layout (location = 0) out vec4 oColor;

void main()
{
    oColor = fCol0 * texture(uSamplerDiffuse, fTex0.st);
}
