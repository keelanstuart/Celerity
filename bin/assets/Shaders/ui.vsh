uniform mat4 uMatrixMVP;

layout (location = 0) in vec2 vPos;
layout (location = 1) in vec2 vTex0;

out vec2 fTex0;

void main()
{
    gl_Position = uMatrixMVP * vec4(vPos.x, 0, vPos.y, 1);
    fTex0 = vTex0;
}
