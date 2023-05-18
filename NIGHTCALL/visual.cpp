#include <windows.h>
#include <math.h>
#include <iostream>
#include "shaders.h"

void DrawMain(int time, float lineWidth);
void DrawGrid(int time);

GLuint bloom;

unsigned int FBO;
unsigned int framebufferTexture;
unsigned int rectVAO, rectVBO;

float rectangleVertices[] =
{
	// Coords    // texCoords
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f,

	 1.0f,  1.0f,  1.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f
};

void InitVisual()
{
	bloom = LoadShader("shaders/bloom.vert", "shaders/bloom.frag");

	glUniform1i(glGetUniformLocation(bloom, "screenTexture"), 0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CCW);
}

void Rescale(int width, int height)
{
	glViewport(0, 0, width, height);
	float ratio = width / (float)height;
	float sz = 1.0;
	glLoadIdentity();
	glFrustum(-ratio * sz, ratio * sz, -sz, sz, 10, 1000);

	glGenVertexArrays(1, &rectVAO);
	glGenBuffers(1, &rectVBO);
	glBindVertexArray(rectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &framebufferTexture);
	glBindTexture(GL_TEXTURE_2D, framebufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);


	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer error: " << fboStatus << std::endl;
}

void DrawScene(int time)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	DrawMain(time, 1.0f);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUseProgram(bloom);

	glBindVertexArray(rectVAO);
	glBindTexture(GL_TEXTURE_2D, framebufferTexture);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	DrawMain(time, 1.0f);
	glDisable(GL_BLEND);

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

void DrawMain(int time, float lineWidth)
{
	glUseProgram(0);
	glPushMatrix();
		glLineWidth(lineWidth);
		glColor3f(189 / 255.0f, 99 / 255.0f, 195 / 255.0f);
		DrawGrid(time);
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