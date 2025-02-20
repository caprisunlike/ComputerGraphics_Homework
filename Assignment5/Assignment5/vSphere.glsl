#version 330

in  vec4 vPosition;
in  vec4 vColor;
in  vec3 vNormal; 
in  vec2 vTexCoord;

out vec2 T2;
out vec4 color;

uniform mat4 uMat;

void main()
{
	
	gl_Position  = uMat*vPosition;
	gl_Position *= vec4(1,1,-1,1);

	color = vColor;
	T2 = vTexCoord;
}
