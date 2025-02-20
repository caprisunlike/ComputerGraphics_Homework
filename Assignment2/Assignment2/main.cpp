#include <vgl.h>
#include <InitShader.h>
#include <mat.h>
#include "MyCube.h"
#include "MyPyramid.h"
#include "MyTarget.h"

MyCube cube;
MyPyramid pyramid;
MyTarget target(&cube);

GLuint program;
GLuint uMat;

mat4 CTM;

bool bPlay = false;
bool bChasingTarget = false;
bool bDrawTarget = false;

float ang1 = 0;
float ang2 = 0;
float ang3 = 0;

void myInit()
{
    cube.Init();
    pyramid.Init();

    program = InitShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);
}

float g_time = 0;

void drawRobotArm(float ang1, float ang2, float ang3)
{
    mat4 temp = CTM;

    // BASE
    mat4 M(1.0);

    M = Translate(0, 0, 0.075) * Scale(0.3, 0.2, 0.05);
    glUniformMatrix4fv(uMat, 1, true, CTM * M);
    pyramid.Draw(program);
    M = Translate(0, 0, -0.075) * Scale(0.3, 0.2, 0.05);
    glUniformMatrix4fv(uMat, 1, true, CTM * M);
    pyramid.Draw(program);

    M = Translate(0, 0.02, 0.115) * Scale(0.03, 0.03, 0.03);
    glUniformMatrix4fv(uMat, 1, true, CTM * M);
    cube.Draw(program);
    M = Translate(0, 0.02, -0.115) * Scale(0.03, 0.03, 0.03);
    glUniformMatrix4fv(uMat, 1, true, CTM * M);
    cube.Draw(program);

    // Upper Arm
    CTM *= RotateZ(ang1);
    M = Translate(0, 0.2, 0) * Scale(0.1, 0.5, 0.1);
    glUniformMatrix4fv(uMat, 1, true, CTM * M);
    cube.Draw(program);

    // Lower Arm
    CTM *= Translate(0, 0.4, 0) * RotateZ(ang2);
    M = Translate(0, 0.2, 0.075) * Scale(0.1, 0.5, 0.05);
    glUniformMatrix4fv(uMat, 1, true, CTM * M);
    cube.Draw(program);
    M = Translate(0, 0.2, -0.075) * Scale(0.1, 0.5, 0.05);
    glUniformMatrix4fv(uMat, 1, true, CTM * M);
    cube.Draw(program);

    M = Translate(0, 0.02, 0.115) * Scale(0.03, 0.03, 0.03);
    glUniformMatrix4fv(uMat, 1, true, CTM * M);
    cube.Draw(program);
    M = Translate(0, 0.02, -0.115) * Scale(0.03, 0.03, 0.03);
    glUniformMatrix4fv(uMat, 1, true, CTM * M);
    cube.Draw(program);

    // Hand
    CTM *= Translate(0, 0.4, 0) * RotateZ(ang3);
    M = Translate(0, 0.1, 0) * Scale(0.3, 0.2, 0.1);
    glUniformMatrix4fv(uMat, 1, true, CTM * M);
    pyramid.Draw(program);

    M = Translate(0, 0.02, 0.115) * Scale(0.03, 0.03, 0.03);
    glUniformMatrix4fv(uMat, 1, true, CTM * M);
    cube.Draw(program);
    M = Translate(0, 0.02, -0.115) * Scale(0.03, 0.03, 0.03);
    glUniformMatrix4fv(uMat, 1, true, CTM * M);
    cube.Draw(program);

    CTM = temp;
}

vec4 computeHandPosition(float ang[3]) {
    mat4 mat = Translate(0, -0.4, 0) * RotateY(g_time * 30);
    mat *= RotateZ(ang[0]);
    mat *= Translate(0, 0.4, 0) * RotateZ(ang[1]);
    mat *= Translate(0, 0.4, 0) * RotateZ(ang[2]);
    mat *= Translate(0, 0.2, 0);
    return mat * vec4(0, 0, 0, 1);
}

float computeDist(vec4 p, vec4 q) {
    return (p.x - q.x) * (p.x - q.x) +
        (p.y - q.y) * (p.y - q.y) +
        (p.z - q.z) * (p.z - q.z);
}

void computeAngle()
{
    float tmp_ang[3];
    tmp_ang[0] = ang1; tmp_ang[1] = ang2; tmp_ang[2] = ang3;

    vec3 tar_pos = target.GetPosition(g_time);
    vec4 hand_pos = computeHandPosition(tmp_ang);
    vec4 targetPoint = Translate(0, -0.4, 0) * RotateY(g_time * 30) * vec4(tar_pos.x, tar_pos.y, 0, 1);

    float min_dist = computeDist(targetPoint, hand_pos);
    if (min_dist < 0.001) return;
    float dist;
    int idx = 0;
    int jdx = 0;

    for (int k = 0; k < 20; k++) {
        idx = 0; jdx = 0;
        for (int i = 0; i < 2; i++) {
            for (int j = -1; j <= 1; j += 2) {
                tmp_ang[i] += j;   // -1도 or +1도
                hand_pos = computeHandPosition(tmp_ang);

                dist = computeDist(targetPoint, hand_pos);
                if (dist < min_dist) {
                    min_dist = dist;
                    idx = i;
                    jdx = j;
                }
                tmp_ang[i] -= j;   // 원래값으로 복귀
            }
        }

        tmp_ang[idx] += jdx * 5;   // jdx 방향으로 5도 이동
        if (tmp_ang[idx] > 180) tmp_ang[idx] -= 360;
        if (tmp_ang[idx] < -180) tmp_ang[idx] += 360;
        hand_pos = computeHandPosition(tmp_ang);
        min_dist = computeDist(targetPoint, hand_pos);

        if (min_dist < 0.001) break;
    }

    ang1 = tmp_ang[0];
    ang2 = tmp_ang[1];
    ang3 = tmp_ang[2];
}

void myDisplay()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    GLuint uColor = glGetUniformLocation(program, "uColor");
    glUniform4f(uColor, -1, -1, -1, -1);

    uMat = glGetUniformLocation(program, "uMat");
    CTM = Translate(0, -0.4, 0) * RotateY(g_time * 30);
    drawRobotArm(ang1, ang2, ang3);

    glUniform4f(uColor, 1, 0, 0, 1);
    if (bDrawTarget == true)
        target.Draw(program, CTM, g_time);

    glutSwapBuffers();
}

void myIdle()
{
    if (bPlay)
    {
        g_time += 1 / 60.0f;
        Sleep(1 / 60.0f * 1000);

        if (bChasingTarget == false)
        {
            ang1 = 45 * sin(g_time * 3.141592);
            ang2 = 60 * sin(g_time * 2 * 3.141592);
            ang3 = 30 * sin(g_time * 3.141592);
        }
        else
            computeAngle();

        glutPostRedisplay();
    }
}

void myKeyboard(unsigned char c, int x, int y)
{
    switch (c)
    {
    case '1':
        bChasingTarget = !bChasingTarget;
        break;
    case '2':
        bDrawTarget = !bDrawTarget;
        break;
    case '3':
        target.toggleRandom();
        break;
    case ' ':
        bPlay = !bPlay;
        break;
    default:
        break;
    }
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutCreateWindow("Simple Robot Arm");

    glewExperimental = true;
    glewInit();

    myInit();
    glutDisplayFunc(myDisplay);
    glutKeyboardFunc(myKeyboard);
    glutIdleFunc(myIdle);

    glutMainLoop();

    return 0;
}