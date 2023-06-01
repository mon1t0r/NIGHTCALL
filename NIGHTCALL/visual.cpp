#define _USE_MATH_DEFINES
#include <windows.h>
#include <cmath>
#include <iostream>
#include "shaders.h"

void DrawBuffer(GLuint program, GLuint frameTexture, GLuint frameBuffer, bool clearBuf);
void BloomAndDrawToMain(int iter, int blurRadius, float blurAlpha, bool clearBuf);
void DrawGrid(int time);
void DrawSun(int time, float r);

GLuint bloom;
GLuint _default;

const int Buffers = 2;

unsigned int FBO[Buffers];
unsigned int FBTex[Buffers];
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
	bloom = LoadShader("shaders/default.vert", "shaders/bloom.frag");
	_default = LoadShader("shaders/default.vert", "shaders/default.frag");

	glUniform1i(glGetUniformLocation(bloom, "screenTexture"), 0);
	glUniform1i(glGetUniformLocation(_default, "screenTexture"), 0);

	glEnable(GL_DEPTH_TEST);
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

	unsigned int RBO[Buffers];

	for (int i = 0; i < Buffers; ++i)
	{
		glGenFramebuffers(1, &FBO[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO[i]);

		glGenTextures(1, &FBTex[i]);
		glBindTexture(GL_TEXTURE_2D, FBTex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBTex[i], 0);

		glGenRenderbuffers(1, &RBO[i]);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO[i]);
	}


	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer error: " << fboStatus << std::endl;
}

void DrawScene(int time)
{
	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO[0]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	DrawSun(time, 0.31f);
	BloomAndDrawToMain(7, 5, 1.0f, true);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO[0]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	DrawGrid(time);
	BloomAndDrawToMain(4, 6, 0.5f, false);

	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	DrawSun(time, 0.3f);
	DrawGrid(time);

	glPushMatrix();
		glLoadIdentity();
		glLineWidth(2.0f);
		glColor3f(189 / 255.0f, 99 / 255.0f, 195 / 255.0f);

		glBegin(GL_LINES);
			glVertex2f(-1.0f, 0.0f);
			glVertex2f(1.0f, 0.0f);
		glEnd();
	glPopMatrix();
}

void BloomAndDrawToMain(int iter, int blurRadius, float blurAlpha, bool clearBuf)
{
	glUseProgram(bloom);
	glUniform1f(glGetUniformLocation(bloom, "alpha"), 1.0f);
	glUniform1i(glGetUniformLocation(bloom, "radius"), blurRadius);
	for (int i = 0; i < iter; ++i)
	{
		if (i % 2)
		{
			glUseProgram(bloom);
			glUniform1i(glGetUniformLocation(bloom, "horizontal"), 0);
			DrawBuffer(bloom, FBTex[1], FBO[0], true);
			continue;
		}
		glUseProgram(bloom);
		glUniform1i(glGetUniformLocation(bloom, "horizontal"), 1);
		DrawBuffer(bloom, FBTex[0], FBO[1], true);
	}

	glUseProgram(_default);
	glUniform1f(glGetUniformLocation(_default, "alpha"), blurAlpha);

	if (iter % 2)
		DrawBuffer(_default, FBTex[1], 0, clearBuf);
	else
		DrawBuffer(_default, FBTex[0], 0, clearBuf);
}

void DrawBuffer(GLuint program, GLuint frameTexture, GLuint frameBuffer, bool clearBuf)
{
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	if (clearBuf)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	}

	glBindVertexArray(rectVAO);
	glBindTexture(GL_TEXTURE_2D, frameTexture);

	glUseProgram(program);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glUseProgram(0);
}

void DrawGrid(int time)
{
	glColor3f(189 / 255.0f, 99 / 255.0f, 195 / 255.0f);
	glPushMatrix();

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
	glPopMatrix();
}

void DrawSun(int time, float r)
{
	int gColMin = 95, gColDelta = 116;

	glPushMatrix();

	glTranslatef(0.0f, 0.5f, -11.0f);

	glBegin(GL_TRIANGLE_FAN);

	glColor3f(255 / 255.0f, (gColMin + 0.5f * gColDelta) / 255.0f, 79 / 255.0f);
	glVertex2f(0.0f, 0.0f);
	for (int i = 0; i <= 360; ++i)
	{
		float sin = sinf(i * M_PI / 180.0f);
		glColor3f(255 / 255.0f, (gColMin + (sin + 1) * 0.5f * gColDelta) / 255.0f, 79 / 255.0f);
		glVertex2f(cosf(i * M_PI / 180.0f) * r, sin * r);
	}

	glEnd();
	glPopMatrix();
}