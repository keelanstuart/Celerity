uniform sampler2D uSamplerDiffuse;

in vec2 fTex0;
in vec4 fColor0;

layout (location = 0) out vec4 oColor;

void main()
{
    oColor = fColor0 * texture(uSamplerDiffuse, fTex0.st);
}
