#include <windows.h>
#include <gl/gl.h>
#include <math.h>

#define FLOAT_MAX 1000.0000000000000000f

void DrawGrid(int, float);

void DrawScene(int time, float xScale)
{
	glPushMatrix();

	glLineWidth(1.0f);
	DrawGrid(time, xScale);

	glPopMatrix();

	glPushMatrix();
	glLoadIdentity();

	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_TRIANGLE_STRIP);

	glVertex2f(-1.0f, 0.0f);
	glVertex2f(1.0f, 0.0f);
	glVertex2f(-1.0f, 1.0f);
	glVertex2f(1.0f, 1.0f);

	glEnd();

	glLineWidth(2.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);

	glVertex2f(-1.0f, 0.0f);
	glVertex2f(1.0f, 0.0f);

	glEnd();

	glPopMatrix();
}

void DrawGrid(int time, float xScale)
{
	glColor3f(1.0f, 1.0f, 1.0f);

	glTranslatef(0.0f, -1.1f, -5.0f);
	glRotatef(-89.3f, 1.0f, 0.0f, 0.0f);

	glBegin(GL_LINES);

	int xCount = 43;
	float xDelta = 0.575f;

	for (int i = -xCount; i <= xCount; ++i)
	{
		glVertex3f(i * xDelta, 9999.0f, 0.0f);
		glVertex3f(i * xDelta, 0.0f, 0.0f);
	}

	float yDelta = 1.5f;
	int yCount = 60;
	float timeDelta = (time % 25) / 25.0f;

	for (int i = -yCount; i < 0; ++i)
	{
		double y = (i + timeDelta) * -yDelta;
		glVertex3f(-999.0, y, 0.0f);
		glVertex3f(999.0, y, 0.0f);
	}

	glEnd();
}