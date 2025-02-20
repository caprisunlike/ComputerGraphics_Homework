
#define _CRT_SECURE_NO_WARNINGS

#include <vgl.h>
#include <InitShader.h>
#include "MyCube.h"
#include "MyUtil.h"

#include <vec.h>
#include <mat.h>
#include <iostream>
#include <queue>
#include <tuple>
using namespace std;

#define MAZE_FILE	"maze.txt"

MyCube cube;
GLuint program;

mat4 g_Mat = mat4(1.0f);
GLuint uMat;
GLuint uColor;

float wWidth = 1000;
float wHeight = 500;

vec3 cameraPos = vec3(0, 0, 0);
vec3 viewDirection = vec3(0, 0, -1);
vec3 goalPos = vec3(0, 0, 0);
vec3 viewAng;


int MazeSize;
char maze[255][255] = { 0 };

float cameraSpeed = 0.1;

float g_time = 0;

int start_x, start_y;
int goal_x, goal_y;
bool findPath = false;
bool collision = false;
bool bRotate = false;
bool bAutoMove = false;


inline vec3 getPositionFromIndex(int i, int j)
{
	float unit = 1;
	vec3 leftTopPosition = vec3(-MazeSize / 2.0 + unit / 2, 0, -MazeSize / 2.0 + unit / 2);
	vec3 xDir = vec3(1, 0, 0);
	vec3 zDir = vec3(0, 0, 1);
	return leftTopPosition + i * xDir + j * zDir;
}

inline pair<int, int> getIndexFromPosition(vec3 position)
{
	float unit = 1;
	vec3 leftTopPosition = vec3(-MazeSize / 2.0 + unit / 2, 0, -MazeSize / 2.0 + unit / 2);

	int i = round(position.x - leftTopPosition.x);
	int j = round(position.z - leftTopPosition.z);

	if (i < 0 || i >= MazeSize || j < 0 || j >= MazeSize)
		return make_pair(-1, -1);  // 유효하지 않은 위치

	return make_pair(i, j);
}

void LoadMaze()
{
	FILE* file = fopen(MAZE_FILE, "r");
	char buf[255];
	fgets(buf, 255, file);
	sscanf(buf, "%d", &MazeSize);
	for (int j = 0; j < MazeSize; j++)
	{
		fgets(buf, 255, file);
		for (int i = 0; i < MazeSize; i++)
		{
			maze[i][j] = buf[i];
			if (maze[i][j] == 'C') {				// Setup Camera Position
				cameraPos = getPositionFromIndex(i, j);
				start_x = i; start_y = j;
				printf("%d %d\n", start_x, start_y);
			}
			if (maze[i][j] == 'G') {				// Setup Goal Position
				goalPos = getPositionFromIndex(i, j);
				goal_x = i; goal_y = j;
				printf("%d %d\n", goal_x, goal_y);
			}
		}
	}
	fclose(file);
}

void DrawMaze()
{
	for (int j = 0; j < MazeSize; j++)
		for (int i = 0; i < MazeSize; i++)
			if (maze[i][j] == '*')
			{
				vec3 color = vec3(i / (float)MazeSize, j / (float)MazeSize, 1);
				mat4 ModelMat = Translate(getPositionFromIndex(i, j));
				glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * ModelMat);
				glUniform4f(uColor, color.x, color.y, color.z, 1);
				cube.Draw(program);
			}
}

void drawPath(vector<pair<int, int>> road) {
	for (int i = road.size()-1; i > 0; i--) {
		float dx = road[i].first - road[i - 1].first;
		float dz = road[i].second - road[i - 1].second;
		vec3 pos = getPositionFromIndex(road[i].first, road[i].second);
		pos.y = -0.5;
		float scale_x = 0.0f, scale_z = 0.0f;
		if (dx == 0) { scale_z = 1.0f; scale_x = 0.1f; pos.z -= dz/2; }
		if (dz == 0) { scale_x = 1.0f; scale_z = 0.1f; pos.x -= dx/2; }
		mat4 M = Translate(pos) * Scale(scale_x, 0.03, scale_z);
		glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * M);
		glUniform4f(uColor, 1, 0, 0, 1);
		cube.Draw(program);
	}
}

int dist(int cx, int cy, int gx, int gy) {
	return abs(cx - gx) + abs(cy - gy);
}

int h[100][100];
void init_h(int gx, int gy) {
	for (int j = 0; j < MazeSize; j++) {
		for (int i = 0; i < MazeSize; i++) {
			if (maze[i][j] == '*') {
				h[i][j] = -5;   // 벽은 -5로 표시
			}
			else {
				h[i][j] = dist(i, j, gx, gy);
			}
		}
	}
}

// 상하좌우
int dx[4] = { 0,0,-1,1 };
int dy[4] = { -1,1,0,0 };

struct Node {
	int f;
	int idx1;
	int idx2;

	bool operator<(const Node& other) const {
		return f > other.f;  // 최소 힙
	}
};

vector<pair<int, int>> path;
void a_star(int sx, int sy, int gx, int gy) {
	int g[100][100];
	int f[100][100];   // f = g + h
	int parent[100][100][2];
	priority_queue<Node> pq;   // f, idx_x, idx_y

	for (int j = 0; j < MazeSize; j++) {
		for (int i = 0; i < MazeSize; i++) {
			g[i][j] = MAXINT;
			f[i][j] = MAXINT;
			parent[i][j][0] = -1;  // 부모 노드 초기화
			parent[i][j][1] = -1;
		}
	}

	g[sx][sy] = 0;
	f[sx][sy] = dist(sx, sy, gx, gy);
	pq.push({ f[sx][sy], sx, sy });

	while (!pq.empty()) {
		Node current = pq.top();
		pq.pop();

		int cx = current.idx1;
		int cy = current.idx2;

		if (cx == gx && cy == gy) {  // 목표 도달
			// 경로 역추적
			int tx = gx, ty = gy;
			while (tx != -1 && ty != -1) {
				path.push_back({ tx, ty });
				int px = parent[tx][ty][0];
				int py = parent[tx][ty][1];
				tx = px;
				ty = py;
			}

			return;
		}

		if (f[cx][cy] == -1) continue;  // 이미 방문한 노드
		f[cx][cy] = -1;  // 방문 표시

		// 인접노드 탐색
		for (int i = 0; i < 4; i++) {
			int tx = cx + dx[i], ty = cy + dy[i];
			if (tx < 0 || tx >= MazeSize || ty < 0 || ty >= MazeSize) continue;
			if (f[tx][ty] == -1) continue;
			if (h[tx][ty] == -5) continue;

			int ng = g[cx][cy] + 1;
			if (ng < g[tx][ty]) {
				g[tx][ty] = ng;
				f[tx][ty] = g[tx][ty] + h[tx][ty];
				parent[tx][ty][0] = cx;  // 부모 노드 저장
				parent[tx][ty][1] = cy;
				pq.push({ f[tx][ty], tx, ty });
			}
		}
	}
	printf("no path\n");  // 경로를 찾지 못한 경우
}

int fidx = 0;
float alpha = 0.0f;
vec3 st, ed;
vec3 sDir;
vec3 eDir;
void followPath() {
	if (!bAutoMove) return;

	alpha += 0.1f;
	if (alpha >= 1.0f) {
		alpha = 0.0f;

		fidx--;
		if (fidx <= 0) {
			bAutoMove = false;
			return;
		}

		st = getPositionFromIndex(path[fidx].first, path[fidx].second);
		ed = getPositionFromIndex(path[fidx - 1].first, path[fidx - 1].second);
		sDir = viewAng;
		eDir = normalize(ed - st);
	}
	
	cameraPos = (1 - alpha) * st + alpha * ed;
	viewAng = normalize((1 - alpha) * sDir + alpha * eDir);
	
}

void myInit()
{
	LoadMaze();
	cube.Init();
	init_h(goal_x, goal_y);   // 휴리스틱 값
	program = InitShader("vshader.glsl", "fshader.glsl");

}

void DrawGrid()
{
	float n = 40;
	float w = MazeSize;
	float h = MazeSize;

	for (int i = 0; i < n; i++)
	{
		mat4 m = Translate(0, -0.5, -h / 2 + h / n * i) * Scale(w, 0.02, 0.02);
		glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * m);
		glUniform4f(uColor, 1, 1, 1, 1);
		cube.Draw(program);
	}
	for (int i = 0; i < n; i++)
	{
		mat4 m = Translate(-w / 2 + w / n * i, -0.5, 0) * Scale(0.02, 0.02, h);
		glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * m);
		glUniform4f(uColor, 1, 1, 1, 1);
		cube.Draw(program);
	}
}

float ang = 0.0f;
void drawCamera()
{
	float cameraSize = 0.5;

	mat4 ModelMat = Translate(cameraPos) * RotateY(ang) * Scale(vec3(cameraSize));
	glUseProgram(program);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * ModelMat);
	glUniform4f(uColor, 0, 1, 0, 1);
	cube.Draw(program);

	ModelMat = Translate(cameraPos) * RotateY(ang) * Translate(viewDirection * cameraSize / 2) * Scale(vec3(cameraSize / 2));
	glUseProgram(program);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * ModelMat);
	glUniform4f(uColor, 0, 1, 0, 1);
	cube.Draw(program);
}

void drawGoal()
{
	glUseProgram(program);
	float GoalSize = 0.7;

	mat4 ModelMat = Translate(goalPos) * RotateY(g_time * 3) * Scale(vec3(GoalSize));
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * ModelMat);
	glUniform4f(uColor, 0, 0, 0, 0);
	cube.Draw(program);

	ModelMat = Translate(goalPos) * RotateY(g_time * 3 + 45) * Scale(vec3(GoalSize));
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * ModelMat);
	glUniform4f(uColor, 0, 0, 0, 0);
	cube.Draw(program);
}

pair<int, int> ind;
void drawCollision() {
	mat4 M = Translate(getPositionFromIndex(ind.first, ind.second)) * Scale(1.1f, 1.1f, 1.1f);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * M);
	glUniform4f(uColor, 1, 0, 0, 1);
	cube.Draw(program);
}


void drawScene(bool bDrawCamera = true)
{
	glUseProgram(program);
	uMat = glGetUniformLocation(program, "uMat");
	uColor = glGetUniformLocation(program, "uColor");

	DrawGrid();
	DrawMaze();
	drawGoal();

	if (bDrawCamera)
		drawCamera();

	if (findPath) {
		drawPath(path);
	}

	if (collision) {
		drawCollision();
	}

}

bool checkCollision(vec3 position, float direction) {
	float margin = 0.2;
	vec3 checkPosition[] = {
		position + vec3(0, 0, direction * margin),
		position + vec3(margin, 0, 0),
		position + vec3(-margin, 0, 0),
	};

	for (int i = 0; i < 3; i++) {
		ind = getIndexFromPosition(checkPosition[i]);

		// 미로 바깥
		if (ind.first < 0 || ind.first >= MazeSize || ind.second < 0 || ind.second >= MazeSize)
			return true;

		// 벽
		if (maze[ind.first][ind.second] == '*') {
			return true;
		}
	}
	return false;
}

void display()
{
	glEnable(GL_DEPTH_TEST);

	float vWidth = wWidth / 2;
	float vHeight = wHeight;

	// LEFT SCREEN : View From Camera (Perspective Projection)
	glViewport(0, 0, vWidth, vHeight);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	float h = 4;
	float aspectRatio = vWidth / vHeight;
	float w = aspectRatio * h;
	if (!bAutoMove) {
		mat4 Ry = RotateY(ang);
		vec4 tempAng = Ry * viewDirection;
		viewAng = vec3(tempAng.x, tempAng.y, tempAng.z);
	}
	else {
		ang = atan2(viewAng.x, viewAng.z) / 3.141592 * 180.0f - 180;
	}
	mat4 ViewMat = myLookAt(cameraPos, cameraPos + viewAng, vec3(0, 1, 0));
	mat4 ProjMat = myPerspective(45, aspectRatio, 0.01, 20);

	g_Mat = ProjMat * ViewMat;
	drawScene(false);							// drawing scene except the camera


	// RIGHT SCREEN : View from above (Orthographic parallel projection)
	glViewport(vWidth, 0, vWidth, vHeight);
	h = MazeSize;
	w = aspectRatio * h;
	ViewMat = myLookAt(vec3(0, 5, 0), vec3(0, 0, 0), vec3(0, 0, -1));
	ProjMat = myOrtho(-w / 2, w / 2, -h / 2, h / 2, 0, 20);

	g_Mat = ProjMat * ViewMat;
	drawScene(true);


	glutSwapBuffers();
}

vec3 newPos;
void idle()
{
	g_time += 1;

	if (bAutoMove && findPath) {
		collision = false;
		followPath();
	}

	if ((GetAsyncKeyState('A') & 0x8000) == 0x8000) {		// if "A" key is pressed	: Go Left
		ang += 5;
	}
	if ((GetAsyncKeyState('D') & 0x8000) == 0x8000) {		// if "D" key is pressed	: Go Right
		ang -= 5;
	}
	if ((GetAsyncKeyState('W') & 0x8000) == 0x8000) {		// if "W" key is pressed	: Go Forward
		newPos = cameraPos + cameraSpeed * viewAng;
		collision = checkCollision(newPos, -1);
		if (!collision)
			cameraPos += cameraSpeed * viewAng;
		else
			cameraPos -= cameraSpeed/2 * viewAng;
	}
	if ((GetAsyncKeyState('S') & 0x8000) == 0x8000) {		// if "S" key is pressed	: Go Backward
		newPos = cameraPos + cameraSpeed * -viewAng;
		collision = checkCollision(newPos, 1);
		if (!collision)
			cameraPos += cameraSpeed * -viewAng;
		else
			cameraPos -= cameraSpeed/2 * -viewAng;
	}

	if ((GetAsyncKeyState('Q') & 0x8000) == 0x8000) {		// if "Q" key is pressed	: Find path
		findPath = true;
		path.clear();
		pair<int, int> cur = getIndexFromPosition(cameraPos);
		a_star(cur.first, cur.second, goal_x, goal_y);
	}
	if ((GetAsyncKeyState(' ') & 0x8000) == 0x8000) {		// if " " key is pressed	: Auto Move
		fidx = path.size() - 1;
		alpha = 0.0f;
		st = getPositionFromIndex(path[fidx].first, path[fidx].second);
		ed = getPositionFromIndex(path[fidx - 1].first, path[fidx - 1].second);
		sDir = normalize(ed - st);
		eDir = sDir;
		viewAng = sDir;
		ang = atan2(viewAng.x, viewAng.z) / 3.141592 * 180.0f - 180;
		bAutoMove = true;
	}

	Sleep(16);											// for vSync
	glutPostRedisplay();
}

void reshape(int wx, int wy)
{
	printf("%d %d \n", wx, wy);
	wWidth = wx;
	wHeight = wy;
	glutPostRedisplay();
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(wWidth, wHeight);

	glutCreateWindow("Homework3 (Maze Navigator)");

	glewExperimental = true;
	glewInit();

	printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION),
		glGetString(GL_SHADING_LANGUAGE_VERSION));

	myInit();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutMainLoop();

	return 0;
}