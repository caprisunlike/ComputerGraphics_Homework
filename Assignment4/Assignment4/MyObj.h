#pragma once

#include <vgl.h>
#include <vec.h>
#include <stdio.h>
#pragma warning(disable:4996)

struct Norm {
	vec4 normSum;
	int cnt;
};

struct OBJvertex {
	vec4 position;
	vec4 color;
	vec4 snormal;   // surface normal
	vec4 vnormal;   // vertex normal
	vec4* addr;     // nbuf address
};

class MyObj {

public:
	OBJvertex* vertices;
	int NumVertices;

	GLuint vao;
	GLuint buffer;
	bool bInitialized;

	double xmin, ymin, zmin;
	double xmax, ymax, zmax;

	MyObj() {
		bInitialized = false;
		NumVertices = 0;
		vertices = NULL;
		xmin = DBL_MAX; ymin = DBL_MAX; zmin = DBL_MAX;
		xmax = DBL_MIN; ymax = DBL_MIN; zmax = DBL_MIN;
	}
	~MyObj() {
		if (vertices != NULL)
			delete[] vertices;
	}

	GLuint Init(const char *filename) {
		if (bInitialized == true) return vao;
		FILE* file;
		char buf[256];
		file = fopen(filename, "r");
		if (!file) {
			printf("File not Found!\n");
			return -1;
		}

		int vcnt = 0, fcnt = 0;
		long vidx = -1, fidx = -1;
		while (fgets(buf, 256, file) != NULL) {
			if (buf[0] == 'v') {
				if (vidx == -1) vidx = ftell(file) - (long)strlen(buf) -1;
				vcnt++;
			}
			else if (buf[0] == 'f') {
				fcnt++;
			}

		}
		//printf("%d %d\n", vcnt, fcnt);
		float x, y, z;
		vec3* position_buf = new vec3[vcnt];
		Norm* nbuf = new Norm[vcnt];
		fseek(file, vidx, SEEK_SET);
		int i = 0;
		while (fgets(buf, 256, file) != NULL) {
			if (sscanf(buf, "v %f %f %f", &x, &y, &z) == 3) {
				if (x < xmin) xmin = x; if (y < ymin) ymin = y; if (z < zmin) zmin = z;
				if (x > xmax) xmax = x; if (y > ymax) ymax = y; if (z > zmax) zmax = z;
				position_buf[i].x = x;
				position_buf[i].y = y;
				position_buf[i].z = z; 
				nbuf[i].normSum = vec3(0);
				i++;
			}
			else break;
		}
		//printf("%d\n", i);
		int cur = 0;
		int a, b, c;
		vec4 color = vec4(0.5, 0.5, 0.5, 1);
		int n;
		NumVertices = fcnt * 3;
		vertices = new OBJvertex[NumVertices];   // vertex 배열 동적할당
		memset(vertices, 0, sizeof(OBJvertex) * NumVertices);
		do  {
			if (sscanf(buf, "f %d %d %d", &a, &b, &c) == 3) {
				// 면 노말
				vec3 p = position_buf[b - 1] - position_buf[a - 1];
				vec3 q = position_buf[c - 1] - position_buf[a - 1];
				vec3 n = cross(p, q);
				n = normalize(n);

				nbuf[a-1].normSum += n; nbuf[a - 1].cnt++;
				nbuf[b-1].normSum += n; nbuf[b - 1].cnt++;
				nbuf[c-1].normSum += n; nbuf[c - 1].cnt++;

				vertices[cur].position = position_buf[a - 1]; vertices[cur].color = color; vertices[cur].snormal = n; vertices[cur].addr = &nbuf[a-1].normSum; cur++;
				vertices[cur].position = position_buf[b - 1]; vertices[cur].color = color; vertices[cur].snormal = n; vertices[cur].addr = &nbuf[b-1].normSum; cur++;
				vertices[cur].position = position_buf[c - 1]; vertices[cur].color = color; vertices[cur].snormal = n; vertices[cur].addr = &nbuf[c-1].normSum; cur++;
			}
		} while (fgets(buf, 256, file) != NULL);
		//printf("%d\n", cur);

		for (int j = 0; j < vcnt; j++) {
			if (nbuf[j].cnt > 0) {
				nbuf[j].normSum /= nbuf[j].cnt;
				nbuf[j].normSum = normalize(nbuf[j].normSum);
			}
		}
		for (int j = 0; j < NumVertices; j++) {
			vertices[j].vnormal = *vertices[j].addr;
		}
		fclose(file);
		delete[] position_buf;
		delete[] nbuf;

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(OBJvertex) * NumVertices, vertices, GL_STATIC_DRAW);

		bInitialized = true;
		return vao;
	}

	void setAttributes(GLuint program) {
		GLuint vPosition = glGetAttribLocation(program, "vPosition");
		glEnableVertexAttribArray(vPosition);
		glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, sizeof(OBJvertex), BUFFER_OFFSET(0));

		GLuint vColor = glGetAttribLocation(program, "vColor");
		glEnableVertexAttribArray(vColor);
		glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, sizeof(OBJvertex), BUFFER_OFFSET(sizeof(vec4)));

		GLuint vNormal = glGetAttribLocation(program, "vNormal");   // vPhong
		glEnableVertexAttribArray(vNormal);
		glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, sizeof(OBJvertex), BUFFER_OFFSET(sizeof(vec4) + sizeof(vec4)));

		GLuint fNormal = glGetAttribLocation(program, "fNormal");   // fPhong
		glEnableVertexAttribArray(fNormal);
		glVertexAttribPointer(fNormal, 4, GL_FLOAT, GL_FALSE, sizeof(OBJvertex), BUFFER_OFFSET(sizeof(vec4) + sizeof(vec4) + sizeof(vec4)));
	}

	void Draw(GLuint program) {
		if (!bInitialized) return;

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		setAttributes(program);

		glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	}

	vec3 getMax() {
		return vec3(xmax, ymax, zmax);
	}

	vec3 getMin() {
		return vec3(xmin, ymin, zmin);
	}
};