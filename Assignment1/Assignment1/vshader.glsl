#version 330

uniform float uTime;
uniform float uA;
uniform float uLambda;
uniform float uW;
uniform bool uWave;

in vec4 vPosition;
in vec4 vColor;

out vec4 color;
out vec4 position;

void main()
{
	// position
	vec4 wavePosition = vPosition;

	if(uWave == true){
		float x = wavePosition.x;
		float y = wavePosition.y;
		float r = sqrt(x*x+y*y);

		float z = uA * exp(-uLambda * r) * sin(uW * r - uTime*5);

		wavePosition.z = z;
	}
	else{
		wavePosition.z = 0.0;
	}


	// color
	vec4 hColor;
	float flag = abs(wavePosition.z);
	if(wavePosition.z < 0) {
		hColor = (1-5*flag)*vColor + 5*flag*vec4(1,0.5,0,1);
	}
	else {
		hColor = (1-5*flag)*vColor + 5*flag*vec4(0,0.5,1,1);
	}


	// rotation
	float ang1 = uTime*30/180.0f*3.141592f;
	float ang2 = 40/180.0f*3.141592f;

	mat4 m1 = mat4(1.0f);
	// z-rotation													
	m1[0][0] = cos(ang1);
	m1[1][0] = -sin(ang1);
	m1[0][1] = sin(ang1);
	m1[1][1] = cos(ang1);


	mat4 m2 = mat4(1.0f);	
	// x-rotation				
	m2[1][1] = cos(ang2);
	m2[2][1] = -sin(ang2);
	m2[1][2] = sin(ang2);
	m2[2][2] = cos(ang2);

	gl_Position = m2*m1*wavePosition;
	gl_Position.w = 1.0f; 

	color = hColor;
	position = vPosition;
}