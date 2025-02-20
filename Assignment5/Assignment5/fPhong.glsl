#version 330
in vec3 N3; 
in vec3 L3; 
in vec3 V3; 
in vec2 T2;
in vec3 wV;
in vec3 wP;
in vec3 wN;

out vec4 fColor;

uniform mat4 uModelMat; 
uniform mat4 uProjMat; 
uniform vec4 uLPos; 
uniform vec4 uAmb; 
uniform vec4 uDif; 
uniform vec4 uSpc; 
uniform float uShininess; 
uniform sampler2D uSphere;
uniform sampler2D uDifMap;

uniform float uTime;
uniform bool ubDif;
uniform float uFPow;

void main()
{
	vec3 N = normalize(N3); 
	vec3 L = normalize(L3); 
	vec3 V = normalize(V3); 
	vec3 H = normalize(V+L); 

    float NL = max(dot(N, L), 0); 
	float VR = pow(max(dot(H, N), 0), uShininess); 

	fColor = uAmb + uDif*NL; 
	fColor.w = 1;	

	vec3 dir = reflect(-wV, wN);

	float pi = 3.141592f;
	float phi = asin(dir.y);
	float theta = atan(dir.z, dir.x);
	vec2 tpos;
	tpos.x = (theta/(2.0*pi) + 0.5);
	tpos.y = phi/pi + 0.5;
	tpos.y = 1-tpos.y;

	vec4 reflectColor = texture(uSphere, tpos);
	if (ubDif) {
		vec4 diffColor = texture(uDifMap, tpos);
		//fColor = uAmb + diffColor;
		fColor = mix(fColor, diffColor, 0.5);
	}
	
	float F = 0.0;
	float ratio = F + (1.0-F)*pow((1.0+dot(wV, wN)), uFPow);
	fColor = mix(fColor, reflectColor, ratio);
	//fColor = vec4(tpos.x, tpos.y, 0.0, 1.0);

}
