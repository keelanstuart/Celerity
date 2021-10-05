uniform mat4 uMatrixP;

layout (location = 0) in vec2 vPos;
layout (location = 1) in vec2 vTex0;
layout (location = 2) in vec4 vColor0;

out vec2 fTex0;
out vec4 fCol0;

void main()
{
    gl_Position = uMatrixP * vec4(vPos.xy, 0, 1);
    fTex0 = vTex0;
    fCol0 = vCol0;
}
