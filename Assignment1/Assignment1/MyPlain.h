#ifndef _MY_PLAIN_H_
#define _MY_PLAIN_H_

#include <vgl.h>
#include <vec.h>

struct MyPlainVertex {
	vec4 position;
	vec4 color;
};

class MyPlain {
public:
	int m_NumDivision;
	int m_NumPoly;		// number of division for a circle(���� ����)
	int m_NumVertex;	// = m_NumPoly * 2(�ﰢ���� ����) * 3(�ﰢ�� �������� ����)

	GLuint m_vao;			// handle for vertex array
	GLuint m_vbo;			// handle for array buffer

	bool m_bInit;
	bool m_bWave;
	bool m_bPlay;

	MyPlain() {
		m_NumDivision;
		m_NumPoly = 0;
		m_NumVertex = 0;

		m_bInit = false;
		m_bWave = false;
		m_bPlay = false;
	}

	void init(int numDivision) {
		if (numDivision < 2) numDivision = 2;
		if (m_bInit == true && numDivision == m_NumDivision) return;

		m_NumDivision = numDivision;
		m_NumPoly = m_NumDivision * m_NumDivision;
		m_NumVertex = m_NumPoly * 2 * 3;

		printf("Division: %d, Num.of Triangles: %d, Num. of Vertices: %d\n", m_NumDivision, m_NumPoly*2, m_NumVertex);

		MyPlainVertex* vertices = new MyPlainVertex[m_NumVertex];
		vec4 color[2] = { vec4(0.3, 0.3, 0.3, 1), vec4(0.5, 0.5, 0.5, 1)};

		int cur = 0;
		int cidx = 0;
		float size = 1.5f;
		float k = size / m_NumDivision;
		float offset = size / 2.0f;
		for (int i = 0; i < m_NumPoly; i++) {

			// �ε����� �̿��� x, z ��ǥ ���
			int x = i % m_NumDivision;           // �׸����� x ��ǥ
			int y = i / m_NumDivision;           // �׸����� y ��ǥ

			if (((x % 2 == 0) && (y % 2 == 0)) || ((x % 2 == 1) && (y % 2 == 1))) cidx = 0;
			else cidx = 1;

			// �� ��ǥ�� k ũ��(�� ũ��)�� ���Ͽ� ���� ��ǥ�� ��ȯ
			float xPos = -offset + x * k;      // x ��ǥ
			float yPos = -offset + y * k;      // y ��ǥ

			vec4 a = vec4(xPos, yPos, 0, 1);         // ���ϴ�
			vec4 b = vec4(xPos + k, yPos, 0, 1);     // ���ϴ�
			vec4 c = vec4(xPos, yPos + k, 0, 1);     // �»��
			vec4 d = vec4(xPos + k, yPos + k, 0, 1); // ����

			// triangle 1
			vertices[cur].position = a; vertices[cur].color = color[cidx]; cur++;
			vertices[cur].position = b; vertices[cur].color = color[cidx]; cur++;
			vertices[cur].position = c; vertices[cur].color = color[cidx]; cur++;

			// triangle 2
			vertices[cur].position = b; vertices[cur].color = color[cidx]; cur++;
			vertices[cur].position = d; vertices[cur].color = color[cidx]; cur++;
			vertices[cur].position = c; vertices[cur].color = color[cidx]; cur++;
		}

		if (m_bInit == false) {   // ���۰� ��� ����� �� ���� ����
			glGenVertexArrays(1, &m_vao);
			glBindVertexArray(m_vao);

			glGenBuffers(1, &m_vbo);
			glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		}
		else {
			glBindVertexArray(m_vao);
			glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		}

		glBufferData(GL_ARRAY_BUFFER, sizeof(MyPlainVertex) * m_NumVertex, vertices, GL_STATIC_DRAW);

		delete[] vertices;   // GPU�� ��� ���� �� CPU�� �ִ� �����ʹ� ����

		m_bInit = true;
	}

	void connectShader(GLuint prog) {
		GLuint vPosition = glGetAttribLocation(prog, "vPosition");
		glEnableVertexAttribArray(vPosition);
		glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, sizeof(MyPlainVertex), BUFFER_OFFSET(0));

		GLuint vColor = glGetAttribLocation(prog, "vColor");
		glEnableVertexAttribArray(vColor);
		glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, sizeof(MyPlainVertex), BUFFER_OFFSET(sizeof(vec4)));
	}

	void draw(GLuint program) {
		glBindVertexArray(m_vao);
		glUseProgram(program);
		connectShader(program);
		glDrawArrays(GL_TRIANGLES, 0, m_NumVertex);
	}

	void increase() {
		int num = m_NumDivision + 1;
		init(num);
	}

	void decrease() {
		int num = m_NumDivision - 1;
		init(num);
	}

};

#endif