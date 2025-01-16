#define GLEW_STATIC
#include "glew.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include "../libs/glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

GLFWwindow* mainWindow = NULL;
int mainWindowHeight = 1080;
int mainWindowWidth = 1920;

const std::string vShader = "shaders/basic.vert";
const std::string fShader = "shaders/basic.frag";
const std::string bumpMap = "../textures/Mega_Bump.tga";
std::vector<GLfloat> landVertices;
std::vector<GLuint> landIndex;
GLuint vbo, vao, ibo;

float camYaw = 0.f;
float camPitch = 0.f;
float camRadius = 10.f;
float yawRad;
float pitchRad;
glm::vec3 camPosition = glm::vec3(0.f, 0.f, 0.f);
glm::vec3 camTargetPos = glm::vec3(0.f, 0.f, 0.f);
glm::vec3 camUp = glm::vec3(0.f, 1.f, 0.f);
glm::vec3 subjectPos = glm::vec3(0.f, 0.f, -15.f);

bool Initialize();
void OnKeyDown(GLFWwindow* window, int key, int scancode, int action, int mode);
void OnMouseMove(GLFWwindow* window, double posX, double posY);
void MoveCamera(float dYaw, float dPitch);
void OnFrameBufferSize(GLFWwindow* window, int width, int height);
void LoadBuffer(GLuint& vbo, GLuint& vao, GLuint& ibo);
GLfloat Quad(int i);
GLuint QuadIndices(int i);


int main()
{
	if (!Initialize())
		return -1;
	
	LoadBuffer(vbo, vao, ibo);

	ShaderProgram shaderProgram;
	shaderProgram.LoadShaders("../shaders/basic.vert", "../shaders/basic.frag");

	while (!glfwWindowShouldClose(mainWindow))
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glm::mat4 model(1.f), view(1.f), projection(1.f);
		MoveCamera(camYaw, camPitch);
		model = glm::translate(model, subjectPos);
		view = glm::lookAt(camPosition, subjectPos, camUp);
		projection = glm::perspective(glm::radians(45.f), (float)mainWindowWidth / (float)mainWindowHeight, 0.1f, 100.f);
		shaderProgram.Use();
		shaderProgram.SetUniform("model", model);
		shaderProgram.SetUniform("view", view);
		shaderProgram.SetUniform("projection", projection);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glfwSwapBuffers(mainWindow);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glfwTerminate();
	return 0;
}

bool Initialize()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4.6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	if (!glfwInit())
		return false;

	mainWindow = glfwCreateWindow(mainWindowWidth, mainWindowHeight, "BumpLand - Bumpmap to Landscape", NULL, NULL);
	if (!mainWindow)
	{
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(mainWindow);
	glfwSetKeyCallback(mainWindow, OnKeyDown);
	glfwSetCursorPosCallback(mainWindow, OnMouseMove);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		return false;
	}

	glClearColor(0.1f, 0.05f, 0.f, 1.f);
	glViewport(0, 0, mainWindowWidth, mainWindowHeight);
	//glEnable(GL_DEPTH_TEST);

	return true;
}


void OnKeyDown(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void OnMouseMove(GLFWwindow* window, double posX, double posY)
{
	static glm::vec2 lastMousePos = glm::vec2(0.f, 0.f);

	if (glfwGetMouseButton(mainWindow, GLFW_MOUSE_BUTTON_LEFT) == 1)
	{
		camYaw -= ((float)posX - lastMousePos.x) * 0.25f;
		camPitch += ((float)posY - lastMousePos.y) * 0.25f;
	}

	if (glfwGetMouseButton(mainWindow, GLFW_MOUSE_BUTTON_RIGHT) == 1)
	{
		float dX = 0.01f * ((float)posX - lastMousePos.x);
		float dY = 0.01f * ((float)posY - lastMousePos.y);
		camRadius += dX - dY;
	}

	lastMousePos.x = (float)posX;
	lastMousePos.y = (float)posY;
}

void MoveCamera(float dYaw, float dPitch)
{
	dPitch = glm::clamp(dPitch, -90.f, 90.f);
	yawRad = glm::radians(dYaw);
	pitchRad = glm::radians(dPitch);

	camPosition.x = subjectPos.x + camRadius * cosf(pitchRad) * sinf(yawRad);
	camPosition.y = subjectPos.y + camRadius * sinf(pitchRad);
	camPosition.z = subjectPos.z + camRadius * cosf(pitchRad) * cosf(yawRad);
}

void OnFrameBufferSize(GLFWwindow* window, int width, int height)
{
	mainWindowWidth = width;
	mainWindowHeight = height;
	glViewport(0, 0, mainWindowWidth, mainWindowHeight);
}

void LoadBuffer(GLuint& vbo, GLuint& vao, GLuint& ibo)
{
	for (int i = 0; i < 12; i++)
		landVertices.push_back(Quad(i));
	for (int i = 0; i < 6; i++)
		landIndex.push_back(QuadIndices(i));

	GLfloat* data = &landVertices[0];
	GLuint* indexData = &landIndex[0];

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, landVertices.size() * sizeof(GLfloat), data, GL_STATIC_DRAW);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, NULL);
	glEnableVertexAttribArray(0);
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, landIndex.size() * sizeof(GLuint), indexData, GL_STATIC_DRAW);
}

GLfloat Quad(int i)
{
	static GLfloat a[] = {
		//  X       Y       Z
			0.5f,   0.5f,   0.f,
			0.5f,   0.5f,   0.f,
			0.5f,   -0.5f,  0.f,
			-0.5f,  -0.5f,  0.f
	};

	return a[i];
}

GLuint QuadIndices(int i)
{
	static GLuint a[] = {
		0, 1, 2,
		0, 2, 3
	};

	return a[i];
}