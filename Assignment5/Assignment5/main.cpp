#define _CRT_SECURE_NO_WARNINGS

#include <vgl.h>
#include <InitShader.h>
#include <MyCube.h>
#include "MySphere.h"
#include "MyObject.h"
#include "Targa.h"

#include <vec.h>
#include <mat.h>
#include <string.h>


MySphere sphere;
MyObject obj;
GLuint phong_prog;
GLuint sphere_prog;

mat4 g_Mat = mat4(1.0f);
float g_Time = 0;
float g_aspect = 1;

float xAng = 2.2f;
float yAng = 0.0f;

bool bFOpen = false;
bool bPlay = false;
int ch_obj = 0;
bool bDif = false;
float FresnelPower = 10.0f;
float fovy = 70.0f;

mat4 myLookAt(vec3 eye, vec3 at, vec3 up)
{
	mat4 V = mat4(1.0f);
	
	up = normalize(up);
	vec3 n = normalize(at - eye);
	float a = dot(up, n);
	vec3 v = normalize(up-a*n);
	vec3 w = cross(n, v);

	V[0] = vec4(w, dot(-w, eye));
	V[1] = vec4(v, dot(-v, eye));
	V[2] = vec4(-n, dot(n, eye));
	
	return V;
}

mat4 myOrtho(float l, float r, float b, float t, float zNear, float zFar)
{
	vec3 center = vec3((l+r)/2, (b+t)/2, -(zNear)/2);
	mat4 T = Translate(-center);
	mat4 S = Scale(2/(r-l), 2/(t-b), -1/(-zNear+zFar));
	mat4 V = S*T;

	return V;
}

mat4 myPerspective(float angle, float aspect, float zNear, float zFar)
{
	float rad = angle*3.141592/180.0f;
	mat4 V(1.0f);
	float h = 2*zFar*tan(rad/2);
	float w = aspect*h;
	mat4 S = Scale(2/w, 2/h, 1/zFar);

	float c = -zNear/zFar;
	
	mat4 Mpt(1.0f);
	Mpt[2] = vec4(0, 0, 1/(c+1), -c/(c+1));
	Mpt[3] = vec4(0, 0, -1, 0);
	

	V = Mpt*S;

	return V;

}


void myInit()
{
	sphere.Init(40,40);
	obj.Init("bunny.obj");

	phong_prog = InitShader("vPhong.glsl", "fPhong.glsl");
	glUseProgram(phong_prog);

	sphere_prog = InitShader("vSphere.glsl", "fSphere.glsl");
	glUseProgram(sphere_prog);

	STGA image;
	char filename[20];
	printf("Input Image Name\n(class1, class2, elephant, glacier, ice, lions, ny, venice, waterfall)\n");
	while (!bFOpen) {
		printf(">> ");
		scanf("%s", filename);
		//strcpy(filename, "venice");
		char sname[50], dname[50];
		strcpy(sname, filename);
		strcat(sname, "_spheremap.tga");
		strcpy(dname, filename);
		strcat(dname, "_diffusemap.tga");

		for (int i = 0; i < 2; i++) {
			if (i == 0) bFOpen = image.loadTGA(sname);
			else if (i == 1) bFOpen = image.loadTGA(dname);

			if (!bFOpen) {
				printf("File Not Found\n");
				break;
			}

			int w = image.width;
			int h = image.height;
			printf("image size = %d, %d\n", w, h);

			GLuint tex;   // handle
			glGenTextures(1, &tex);

			if (i == 0) glActiveTexture(GL_TEXTURE0);
			else if (i == 1) glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, tex);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
	}
	
}

void display()
{
	glClearColor(0,0,0,1);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
		
	vec3 ePos = vec3(4*cos(xAng*3.141562/5)*cos(yAng*3.141592/5), 4*sin(yAng*3.141592/5), 4*sin(xAng*3.141562/5)*cos(yAng*3.141592/5));

	mat4 ModelMat = RotateY(15*g_Time);
	mat4 ViewMat = myLookAt(ePos, vec3(0,0,0), vec3(0,1,0));
	mat4 ProjMat = myPerspective(fovy, g_aspect, 0.01, 100.0f);

	g_Mat = ProjMat*ViewMat*ModelMat;

	glUseProgram(sphere_prog);
	GLuint uMat = glGetUniformLocation(sphere_prog, "uMat");
	GLuint uSphere = glGetUniformLocation(sphere_prog, "uSphere");
	glUniformMatrix4fv(uMat, 1, GL_TRUE, ProjMat * ViewMat * Scale(10, 10, 10));
	glUniform1i(uSphere, 0);

	sphere.Draw(sphere_prog);
	


	glUseProgram(phong_prog);
	
	// 1. Define Light Properties
	vec4 lPos = vec4(2, 2, 0, 1);			 
	vec4 lAmb = vec4(0.5, 0.5, 0.5, 1);		
	vec4 lDif = vec4(1, 1, 1, 1);
	vec4 lSpc = lDif; 

	// 2. Define Material Properties
	vec4 mAmb = vec4(0.3, 0.3, 0.3, 1);		
	vec4 mDif = vec4(1.0, 1.0, 1.0, 1);		// °ø »ö±ò
	vec4 mSpc = vec4(1.3, 1.3, 1.3, 1); 
	float mShiny = 50;										//1~100;

	// I = lAmb*mAmb + lDif*mDif*(N¡¤L) + lSpc*mSpc*(V¡¤R)^n; 
	vec4 amb = lAmb*mAmb;					
	vec4 dif = lDif*mDif;					
	vec4 spc = lSpc*mSpc; 

	// 3. Send Uniform Variables to the shader
	GLuint uModelMat = glGetUniformLocation(phong_prog, "uModelMat"); 
	GLuint uViewMat = glGetUniformLocation(phong_prog, "uViewMat"); 
	GLuint uProjMat = glGetUniformLocation(phong_prog, "uProjMat"); 
	GLuint uLPos = glGetUniformLocation(phong_prog, "uLPos"); 
	GLuint uAmb = glGetUniformLocation(phong_prog, "uAmb"); 
	GLuint uDif = glGetUniformLocation(phong_prog, "uDif"); 
	GLuint uSpc = glGetUniformLocation(phong_prog, "uSpc"); 
	GLuint uShininess = glGetUniformLocation(phong_prog, "uShininess"); 
	GLuint uTime = glGetUniformLocation(phong_prog, "uTime");
	uSphere = glGetUniformLocation(phong_prog, "uSphere");
	GLuint uDifMap = glGetUniformLocation(phong_prog, "uDifMap");
	GLuint uEPos = glGetUniformLocation(phong_prog, "uEPos");
	GLuint ubDif = glGetUniformLocation(phong_prog, "ubDif");
	GLuint uFPow = glGetUniformLocation(phong_prog, "uFPow");


	glUniformMatrix4fv(uModelMat, 1, true, ModelMat); 
	glUniformMatrix4fv(uViewMat, 1, true, ViewMat); 
	glUniformMatrix4fv(uProjMat, 1, true, ProjMat); 
	glUniform4f(uLPos, lPos[0], lPos[1], lPos[2], lPos[3]); 
	glUniform4f(uAmb, amb[0], amb[1], amb[2], amb[3]); 
	glUniform4f(uDif, dif[0], dif[1], dif[2], dif[3]); 
	glUniform4f(uSpc, spc[0], spc[1], spc[2], spc[3]); 
	glUniform1f(uShininess, mShiny); 

	glUniform1f(uTime, g_Time);
	glUniform1i(uSphere, 0);
	glUniform1i(uDifMap, 1);
	glUniform4f(uEPos, ePos[0], ePos[1], ePos[2], 1);
	glUniform1i(ubDif, bDif);
	glUniform1f(uFPow, FresnelPower);


	if (ch_obj == 1) sphere.Draw(phong_prog);
	else if(ch_obj == 2) obj.Draw(phong_prog);
	
	glutSwapBuffers();
}

void idle()
{
	if (bPlay) {
		g_Time += 0.016;
	}
	Sleep(16);
	glutPostRedisplay();
}
int rx, ry;
int zy;
int mouse_state = 0;

void myMouse(int button, int state, int x, int y) {
	if (state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON) {
			mouse_state = 1;
			rx = x;
			ry = y;
		}
		else if (button == GLUT_RIGHT_BUTTON) {
			mouse_state = 2;
			zy = y;
		}
	}
	if (state == GLUT_UP) {
		if (button == GLUT_LEFT_BUTTON) {
			mouse_state = 0;
		}
		else if (button == GLUT_RIGHT_BUTTON) {
			mouse_state = 0;
		}
	}
}

void myMotion(int x, int y) {
	if (mouse_state == 1) {
		xAng += (rx - x) * 0.005f;
		yAng += (ry - y) * 0.005f;
		if (yAng > 1.5f) yAng = 1.5f;
		if (yAng < -1.5f) yAng = -1.5f;

		rx = x;
		ry = y;
	}
	else if (mouse_state == 2) {
		fovy += (y - zy) * 0.1f;
		if (fovy > 130.0f) fovy = 130.0f;
		if (fovy < 45.0f) fovy = 45.0f;

		zy = y;
	}
	

	glutPostRedisplay();
}

void keyboard(unsigned char ch, int x, int y)
{
	if (ch == ' ')
		bPlay = !bPlay;
	if (ch == 'q') {
		ch_obj += 1;
		if (ch_obj >= 3) {
			ch_obj = 0;
		}
	}
	if (ch == '1') {
		FresnelPower -= 0.5f;
		if (FresnelPower < 0.5f) FresnelPower = 0.5f;
		printf("FresnelPower = %.1f\n", FresnelPower);
	}
	if (ch == '2') {
		FresnelPower += 0.5f;
		if (FresnelPower > 100.0f) FresnelPower = 100.0f;
		printf("FresnelPower = %.1f\n", FresnelPower);
	}
	if (ch == '3') {
		bDif = !bDif;
		if (bDif) {
			printf("Diffuse Map On\n");
		}
		else {
			printf("Diffuse Map Off\n");
		}
	}
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	
	g_aspect = w/float(h);
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
	glutInitWindowSize(800,512);

	glutCreateWindow("Shader Test");

	glewExperimental = true;
	glewInit();

	printf("OpenGL %s, GLSL %s\n",	glGetString(GL_VERSION),
									glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("---------------------------------------------------------\n");
	printf("Left Mouse Button Dragging  : rotating the viewpoint\n");
	printf("Right Mouse Button Dragging : zoom-in/out\n");
	printf("\n");
	printf("'1' key  : Decreasing Fresnel power parameter for shading\n");
	printf("'2' key  : Increasing Fresnel power parameter for shading\n");
	printf("'3' key  : Turn on/off the diffuse map\n");
	printf("'q' key  : toggling between Sphere Model and Bunny Model\n");
	printf("Spacebar : start/stop rotating the model\n");
	printf("---------------------------------------------------------\n");
	
	myInit();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutMouseFunc(myMouse);
	glutMotionFunc(myMotion);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	return 0;
}
