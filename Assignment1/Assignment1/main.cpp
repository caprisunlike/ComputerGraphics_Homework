#include <vgl.h>
#include <InitShader.h>
#include <vec.h>
#include "MyPlain.h"

GLuint program;
MyPlain plain;

void myInit()
{
	plain.init(30);

	program = InitShader("vshader.glsl", "fshader.glsl");
}

float myTime = 0;
float A = 0.4f;        // 초기 진폭
float lambda = 5.0f;   // 감쇠율
float w = 30.0f;       // 파동의 주기
bool bPlay = false;
bool bWave = false;

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(program);

	GLuint uTime = glGetUniformLocation(program, "uTime");
	glUniform1f(uTime, myTime);
	GLuint uA = glGetUniformLocation(program, "uA");
	glUniform1f(uA, A);
	GLuint uLambda = glGetUniformLocation(program, "uLambda");
	glUniform1f(uLambda, lambda);
	GLuint uW = glGetUniformLocation(program, "uW");
	glUniform1f(uW, w);
	GLuint uWave = glGetUniformLocation(program, "uWave");
	glUniform1i(uWave, bWave);

	plain.draw(program);
	glFlush();
}

void idle()
{
	if (bPlay) {
		myTime += 0.0333f;
	}
	Sleep(33);
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case '1': 	plain.decrease();						break;
	case '2': 	plain.increase();						break;
	case 'w':	bWave = !bWave;		break;
	case ' ':	bPlay = !bPlay;		break;
	case 'Q':	exit(0);								break;
	default:											break;
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(700, 700);
	glutCreateWindow("Waving Plain");

	glewExperimental = true;
	glewInit();

	printf("----------------------------------------------\n");
	printf("'1' key: Decreaing the Number of Division\n");
	printf("'2' key: Increaing the Number of Division\n");
	printf("'w' key: Showing/hiding the waving pattern\n");
	printf("Spacebar: starting/stoping rotating and waving\n");
	printf("\n");
	printf("'Q' Key: Exit the program.\n");
	printf("----------------------------------------------\n");
	printf("\n");

	myInit();

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);

	glutMainLoop();

	return 0;
}