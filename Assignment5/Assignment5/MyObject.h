#pragma once 
#include <stdio.h>
#include <vec.h>
#include <vgl.h>

struct face
{
	int vind[3];
	vec3 nor;
};

struct MyObjectVertex
{
	vec4 pos;
	vec3 nor;
};

class MyObject 
{
public:
	MyObject(){}; 
	~MyObject(){};

	int m_NumVertices;		

	GLuint vao; 
	GLuint buffer; 

	void Init(const char * filename); 
	void Draw(GLuint program); 
};

void MyObject::Init(const char* filename)
{
	FILE* file;
	file = fopen(filename, "r");

	if (file == NULL)
	{
		printf("Incorrect filename!");
		return;
	}

	int vNum = 0;
	int fNum = 0;
	while (true)
	{
		char buf[255];
		if (fgets(buf, 255, file) == 0)	break;
		if (buf[0] == 'v')	vNum++;
		if (buf[0] == 'f')	fNum++;
	}

	vec3* pos = new vec3[vNum];
	face* tri = new face[fNum];

	fclose(file);
	file = fopen(filename, "r");
	int vind = 0;
	int find = 0;
	while (true)
	{
		char buf[255];
		if (fgets(buf, 255, file) == 0)	break;
		if (buf[0] == 'v')
		{
			char ch;
			float x, y, z;
			sscanf(buf, "%c %f %f %f", &ch, &x, &y, &z);
			pos[vind] = vec3(x, y, z);
			vind++;
		}
		if (buf[0] == 'f')
		{
			char ch;
			int a, b, c;
			sscanf(buf, "%c %d %d %d", &ch, &a, &b, &c);
			tri[find].vind[0] = a - 1;
			tri[find].vind[1] = b - 1;
			tri[find].vind[2] = c - 1;
			find++;
		}
	}

	fclose(file);

	float xmin = FLT_MAX, xmax = FLT_MIN;
	float ymin = FLT_MAX, ymax = FLT_MIN;
	float zmin = FLT_MAX, zmax = FLT_MIN;

	for (int i = 0; i < vNum; i++)
	{
		if (pos[i].x < xmin) xmin = pos[i].x;
		if (pos[i].x > xmax) xmax = pos[i].x;
		if (pos[i].y < ymin) ymin = pos[i].y;
		if (pos[i].y > ymax) ymax = pos[i].y;
		if (pos[i].z < zmin) zmin = pos[i].z;
		if (pos[i].z > zmax) zmax = pos[i].z;
	}
	vec3 cen((xmax + xmin) / 2, (ymax + ymin) / 2, (zmax + zmin) / 2);
	float leng_x = xmax - xmin;
	float leng_y = ymax - ymin;
	float leng_z = zmax - zmin;
	float leng = leng_x;
	if (leng < leng_y) leng = leng_y;
	if (leng < leng_z) leng = leng_z;
	for (int i = 0; i < vNum; i++)
	{
		pos[i] -= cen;
		pos[i] /= (leng / 2);
	}

	//face normal 구하기 
	for (int i = 0; i < fNum; i++)
	{
		vec3 p[3];
		for (int j = 0; j < 3; j++)
			p[j] = pos[tri[i].vind[j]];
		vec3 a = p[1] - p[0];
		vec3 b = p[2] - p[0];
		vec3 n = normalize(cross(a, b));
		tri[i].nor = n;
	}

	//vertex normal 구하기 
	vec3* nor = new vec3[vNum];
	int* count = new int[vNum];
	for (int i = 0; i < vNum; i++)
	{
		nor[i] = vec3(0, 0, 0);
		count[i] = 0;
	}

	for (int j = 0; j < fNum; j++)
	{
		for (int k = 0; k < 3; k++)
		{
			int vind = tri[j].vind[k];
			nor[vind] += tri[j].nor;
			count[vind] ++;
		}
	}

	for (int i = 0; i < vNum; i++)
	{
		nor[i] /= float(count[i]);
		nor[i] = normalize(nor[i]);
	}

	m_NumVertices = fNum * 3;

	MyObjectVertex* v = new MyObjectVertex[m_NumVertices];
	for (int i = 0; i < fNum; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			v[3 * i + j].pos = pos[tri[i].vind[j]];
			v[3 * i + j].nor = nor[tri[i].vind[j]];
		}
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(MyObjectVertex) * m_NumVertices,
		v, GL_STATIC_DRAW);

	delete[] pos;
	delete[] tri;
	delete[] nor;
	delete[] count;
	delete[] v;
}

void MyObject::Draw(GLuint program)
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, sizeof(MyObjectVertex), BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, sizeof(MyObjectVertex), BUFFER_OFFSET(sizeof(vec4)));

	glDrawArrays(GL_TRIANGLES, 0, m_NumVertices);
}