layout (location=0) in vec4 vPos;
layout (location=1) in vec2 vTex0;

out vec2 fTex0;

void main()
{
	gl_Position = vPos;
    fTex0 = vTex0;
}
