#version 330

in vec2 T2;
in vec4 color;

out vec4 fColor;

uniform sampler2D uSphere;

void main()
{
	fColor = color;
	vec2 dir;
	//dir.x = 1-T2.x;
	dir.x = T2.x;
	dir.y = T2.y;
	vec4 texColor = texture(uSphere, dir);

	fColor = texColor;
	fColor.w = 1;
}
