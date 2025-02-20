#version 330

in  vec4 vPosition;
in  vec4 vColor;
in  vec4 vNormal;
in  vec4 fNormal;

out vec4 Color;
out vec4 Pos;
out vec3 Normal;

uniform mat4 uProjMat;
uniform mat4 uModelMat;
uniform float uShin;
uniform float uSpec;
uniform bool uPhong;

void main()
{
	gl_Position  = uProjMat * (uModelMat * vPosition);
	gl_Position *= vec4(1,1,-1,1);	// z축 방향이 반대임
		   	
	Pos = uModelMat*vPosition;		

	vec4 N;

	if (uPhong) {
		N = uModelMat * vec4(fNormal.xyz,0);
		Normal = normalize(N.xyz);
		Color = vColor;
	}
	else {
		N = uModelMat * vec4(vNormal.xyz,0);
		Normal = normalize(N.xyz);

		vec4 Lpos = vec4(0,1,0,1);			// light position in camera coord.
		vec4 I = vec4(1,1,1,1);
		vec4 ka = vec4(0.1,0.1,0.1,1);	// ambient reflection coeff. 
		vec4 kd = vec4(0.7,0.7,0.9,1);	// diffuse reflection coeff.
		vec4 ks = vec4(uSpec,uSpec,uSpec,1);
	
		vec4 vPos = vec4(0,0,0,1);

		vec3 L3 = normalize((Lpos - Pos).xyz);
		vec3 N3 = normalize(Normal);
		vec3 R3 = 2*dot(L3, N3)*N3 - L3;	// L3 + R3 = 2*dot(L3, N3)*N3
		vec3 V3 = normalize((vPos - Pos).xyz);
	
		vec4 amb = ka*I;
		vec4 diff = kd*I*max(dot(N3, L3),0);
		vec4 spec = ks*I*pow(max(dot(R3, V3),0), uShin);

		Color = amb + diff + spec;

	}
}
