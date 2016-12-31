#version 150 

in vec4 vPosition;
out vec4 pos;
uniform float rot;

vec4 rotatez(float angle, vec4 p)
{
	float r = angle * 3.1415/180.0;
	return vec4(p.x * cos(r) - p.y * sin(r), p.x * sin(r) + p.y * cos(r), p.z, p.w);
}

vec4 rotatey(float angle, vec4 p)
{
	float r = angle * 3.1415/180.0;
	return vec4(p.z * sin(r) + p.x * cos(r), p.y, p.z * cos(r) - p.x * sin(r), p.w);
}

vec4 project(float n, vec4 p)
{
	float zn = n / p.z;
	return vec4(p.x * zn, p.y * zn, n,  p.w);
}

vec4 translate(vec3 t, vec4 p)
{
	return vec4(t.x + p.x, t.y + p.y, t.z + p.z, p.w);
}

void main()
{
	pos = translate(vec3(0, 0, -1), vec4(0.5, 0.5, 0.5, 1) * rotatey(rot, vPosition));
	vec4 new_pos = project(0.9, pos);
	gl_Position = new_pos;
}
