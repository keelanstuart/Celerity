uniform mat4 uMatrixV;
uniform mat4 uMatrixVP;
uniform vec3 uEyePosition;

layout (points) in;
in vec3 gPos[];
in float gSize[];
in float gRoll[];
in vec4 gColor0[];

layout (triangle_strip, max_vertices = 4) out;
out vec4 fPosDepth;
out vec2 fTex0;
out vec4 fColor0;

mat4 createRollMatrix(vec3 axis, float angle)
{
	float c = cos(angle);
	float s = sin(angle);
	float t = 1.0 - c;
	float x = axis.x, y = axis.y, z = axis.z;
	float tx = t * x, ty = t * y;

	return mat4(
		tx * x + c,     tx * y - s * z, tx * z + s * y, 0.0,
		tx * y + s * z, ty * y + c,     ty * z - s * x, 0.0,
		tx * z - s * y, ty * z + s * x, t * z * z + c,  0.0,
		0.0,            0.0,            0.0,            1.0
		);
}

void main (void)
{
	// billboard
	vec3 toCam = normalize(uEyePosition - gPos[0]);
	vec3 up = vec3(uMatrixV[0][1], uMatrixV[1][1], uMatrixV[2][1]);
	vec3 right = cross(toCam, up);
	up = cross(right, toCam);
	
	// roll
	mat4 matRoll = createRollMatrix(toCam, gRoll[0]);

	// lookup tables for quad corners and texture coords
	vec2 ofs[4] = { vec2(-0.5, -0.5), vec2(-0.5, 0.5), vec2(0.5, -0.5), vec2(0.5, 0.5) };
	vec2 uv[4] = { vec2(0.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 0.0), vec2(1.0, 1.0) };

	// generate quad verts
	for (int i = 0; i < 4; ++i)
	{
		vec4 wpos = vec4(gPos[0] + ((right * ofs[i].x) + (up * ofs[i].y)) * gSize[0], 1.0);
		vec4 zpos = uMatrixVP * vec4((matRoll * wpos).xyz, 1.0);

		gl_Position = zpos;

		fPosDepth = vec4(wpos.xyz, zpos.w);
		fTex0 = uv[i];
		fColor0 = gColor0[0];

		EmitVertex();
	}
	
	EndPrimitive();
}