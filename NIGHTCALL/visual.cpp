#include <windows.h>
#include <gl/gl.h>
#include <math.h>

void DrawGrid(int time);

void DrawScene(int time)
{
	glPushMatrix();
		glLineWidth(1.0f);
		glColor3f(189 / 255.0f, 99 / 255.0f, 195 / 255.0f);
		DrawGrid(time);
	glPopMatrix();

	glPushMatrix();
		glLoadIdentity();
		glLineWidth(2.0f);
		glColor3f(1.0f, 0.0f, 0.0f);

		glBegin(GL_LINES);
			glVertex2f(-1.0f, 0.0f);
			glVertex2f(1.0f, 0.0f);
		glEnd();
	glPopMatrix();
}

void DrawGrid(int time)
{
	glTranslatef(0.0f, -1.1f, -5.0f);
	glRotatef(-89.3f, 1.0f, 0.0f, 0.0f);

	glBegin(GL_LINES);

	int xCount = 43;
	float xDelta = 0.575f;

	for (int i = -xCount; i <= xCount; ++i)
	{
		glVertex3f(i * xDelta, 89.5f, 0.0f);
		glVertex3f(i * xDelta, 0.0f, 0.0f);
	}

	int yCount = 60;
	float yDelta = 1.5f;
	float timeDelta = (time % 18) / 18.0f;

	for (int i = -yCount; i < 0; ++i)
	{
		double y = (i + timeDelta) * -yDelta;
		glVertex3f(-999.0, y, 0.0f);
		glVertex3f(999.0, y, 0.0f);
	}

	glEnd();
}