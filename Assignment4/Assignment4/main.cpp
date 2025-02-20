#include <vgl.h>
#include <InitShader.h>
#include "MyCube.h"
#include "MySphere.h"
#include "MyObj.h"

#include <vec.h>
#include <mat.h>
using namespace std;

MyCube cube;
MySphere sphere;
MyObj obj;

GLuint program;
GLuint prog_phong;

GLuint uMat;
GLuint uColor;
mat4 g_Mat = mat4(1.0f);
mat4 ModelMat;

int winWidth = 500;
int winHeight = 500;
float g_Time = 0;

bool bPlay = false;
int axis = 1;
float angX = 0.0f, angY = 0.0f, angZ = 0.0f;
float sh = 50.0;
float sp = 0.5;
bool bPhong = true;

vec3 maxPos = 0;
vec3 minPos = 0;

mat4 myLookAt(vec3 eye, vec3 at, vec3 up)
{
	mat4 V(1.0f);
	vec3 n = at-eye;
	n /= length(n);

	float a = dot(up, n);
	vec3 v = up - a*n;
	v /= length(v);

	vec3 w = cross(n, v);

	mat4 Rw(1.0f);

	Rw[0][0] = w.x;	Rw[0][1] = v.x; Rw[0][2] = -n.x;
	Rw[1][0] = w.y;	Rw[1][1] = v.y; Rw[1][2] = -n.y;
	Rw[2][0] = w.z;	Rw[2][1] = v.z; Rw[2][2] = -n.z;

	mat4 Rc(1.0f);
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			Rc[i][j] = Rw[j][i];

	mat4 Tc = Translate(-eye.x, -eye.y, -eye.z);

	V = Rc*Tc;
		
	return V;
}

mat4 myOrtho(float l, float r, float b, float t, float zNear, float zFar)
{
	mat4 V(1.0f);

	V[0][0] = 2/(r-l);
	V[1][1] = 2/(t-b);
	V[2][2] = 2/(zFar-zNear);
	V[0][3] = -(r+l)/(r-l);
	V[1][3] = -(t+b)/(t-b);
	V[2][3] = (zNear)/(zFar-zNear);

	return V;
}

mat4 myPerspective(float fovy, float aspectRatio, float zNear, float zFar)
{
	mat4 P(1.0f);
	
	float rad = fovy * 3.141592 / 180.0;
	
	float sz = 1 / zFar;
	float h = zFar * tan(rad/2);
	
	float sy = 1/h;
	float w = h*aspectRatio;
	float sx = 1/w;

	mat4 S = Scale(sx, sy, sz);
	mat4 M(1.0f);

	float c = -zNear / zFar;
	M[2][2] = 1/(c+1);
	M[2][3] = -c/(c+1);
	M[3][2] = -1;
	M[3][3] = 0;

	P = M*S;

	return P;
}

int f = -1;
void myInit()
{
	cube.Init();
	//obj.Init("cube.obj");
	//obj.Init("bunny.obj");
	//obj.Init("buddha.obj");
	//obj.Init("dragon.obj");

	char str[20];
	while (f == -1) {
		printf("Input File path : ");
		scanf("%s", str);
		f = obj.Init(str);
	}
	

	maxPos = obj.getMax();
	minPos = obj.getMin();

	program = InitShader("vshader.glsl", "fshader.glsl");
	prog_phong = InitShader("vphong.glsl", "fphong.glsl");
}

void DrawAxis()
{
	mat4 RotMat = RotateZ(angZ) * RotateY(angY) * RotateX(angX);
	glUseProgram(program);
	mat4 x_a= RotMat * Translate(0.5,0,0)*Scale(1,0.01,0.01);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat*x_a);
	glUniform4f(uColor, 1, 0, 0, 1);
	cube.Draw(program);

	mat4 y_a= RotMat * Translate(0, 0.5,0)*Scale(0.01,1,0.01);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat*y_a);
	glUniform4f(uColor, 0, 1, 0, 1);
	cube.Draw(program);

	mat4 z_a= RotMat * Translate(0,0, 0.5)*Scale(0.01,0.01,1);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat*z_a);
	glUniform4f(uColor, 0, 0, 1, 1);
	cube.Draw(program);
}

void display()
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
		
	mat4 ViewMat = myLookAt(vec3(0,0,1.8), vec3(0,0,0), vec3(0,1,0));

	float aspect = winWidth/(float)winHeight;
	float h = 1;

	mat4 ProjMat = myPerspective(50, aspect,0.1,100);

	g_Mat = ProjMat*ViewMat;
	
	glUseProgram(program);
	uMat = glGetUniformLocation(program, "uMat");
	uColor = glGetUniformLocation(program, "uColor");

	DrawAxis();

	glUseProgram(prog_phong);
	GLuint uProjMat = glGetUniformLocation(prog_phong, "uProjMat");
	GLuint uModelMat = glGetUniformLocation(prog_phong, "uModelMat");
	GLuint uShin = glGetUniformLocation(prog_phong, "uShin");
	GLuint uSpec = glGetUniformLocation(prog_phong, "uSpec");
	GLuint uPhong = glGetUniformLocation(prog_phong, "uPhong");

	vec3 len = vec3((maxPos.x-minPos.x), (maxPos.y-minPos.y), (maxPos.z-minPos.z));
	float scaleMax = std::max({ len.x, len.y, len.z });
	scaleMax = 1 / scaleMax;
	vec3 cen = vec3((maxPos.x+minPos.x) / 2, (maxPos.y+minPos.y) / 2, (maxPos.z+minPos.z) / 2);

	ModelMat = Scale(scaleMax) * RotateZ(angZ) * RotateY(angY) * RotateX(angX) * Translate(-cen);
	glUniformMatrix4fv(uProjMat, 1, GL_TRUE, ProjMat);
	glUniformMatrix4fv(uModelMat, 1, GL_TRUE, ViewMat * ModelMat);
	glUniform1f(uShin, sh);
	glUniform1f(uSpec, sp);
	glUniform1i(uPhong, bPhong);

	obj.Draw(prog_phong);
	
	glutSwapBuffers();
}


void idle()
{
	if (bPlay) {
		g_Time += 0.016;

		if (axis == 0) { angX += 1; }
		if (axis == 1) { angY += 1; }
		if (axis == 2) { angZ += 1; }
	}


	Sleep(16);					// for vSync
	glutPostRedisplay();
}

void myMouse(int button, int state, int x, int y) {
	if (state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON) {
			printf("X-rotation\n");
			axis = 0;
		}
		else if (button == GLUT_MIDDLE_BUTTON) {
			printf("Y-rotation\n");
			axis = 1;
		}
		else if (button == GLUT_RIGHT_BUTTON) {
			printf("Z-rotation\n");
			axis = 2;
		}
	}
}

void myKeyboard(unsigned char c, int x, int y)
{
	switch (c)
	{
	case ' ':    // start or stop
		printf("Start or Stop\n");
		bPlay = !bPlay;
		break;
	case '1':    // vertex normal
		printf("Using vertex normal\n");
		bPhong = true;
		break;
	case '2':    // surface normal
		printf("Using surface normal\n");
		bPhong = false;
		break;
	case '3':    // increase specular effect
		sp += 0.05;
		if (sp > 1.0) sp = 1.0;
		printf("Increase Specular effect\n");
		break;
	case '4':    // decrease specular effect
		sp -= 0.05;
		if (sp < 0.0) sp = 0.0;
		printf("Decrease Specular effect\n");
		break;
	case '5':    // increase shininess
		sh -= 5;
		if (sh < 5.0) sh = 5.0;
		printf("Increase Shininess\n");
		break;
	case '6':    // decrease shininess
		sh += 5;
		if (sh > 100.0) sh = 100.0;
		printf("Decrease Shininess\n"); 
		break;
	case 'Q':    // exit
		exit(0);
		break;
	default:
		break;
	}
}

void reshape(int w, int h)
{
	winWidth = w;
	winHeight = h;
	glViewport(0,0,w,h);
	glutPostRedisplay();	
}


int main(int argc, char ** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
	glutInitWindowSize(winWidth,winHeight);

	glutCreateWindow("OpenGL");

	glewExperimental = true;
	glewInit();

	printf("OpenGL %s, GLSL %s\n",	glGetString(GL_VERSION),
									glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("--------------------------------------------\n");
	printf("Spacebar : starting/stoping rotation\n");
	printf("Left Mouse Button   : rotating around x-axis\n");
	printf("Middle Mouse Button : rotating around y-axis\n");
	printf("Right Mouse Button  : rotating around z-axis\n");
	printf("\n");
	printf("'1' key : Using Vertex Normal for shading\n");
	printf("'2' key : Using Surface Normal for shading\n");
	printf("'3' key : Increasing Specular effect (ks)\n");
	printf("'4' key : Decreasing Specular effect (ks)\n");
	printf("'5' key : Increasing Shininess (n)\n");
	printf("'6' key : Decreasing Shininess (n)\n");
	printf("\n");
	printf("'Q' Key : Exit the program.\n");
	printf("--------------------------------------------\n");

	myInit();
	glutDisplayFunc(display);
	glutMouseFunc(myMouse);
	glutKeyboardFunc(myKeyboard);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutMainLoop();

	return 0;
}
