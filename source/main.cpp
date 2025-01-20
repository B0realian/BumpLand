#define GLEW_STATIC
#include "glew.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "../libs/glm/gtc/matrix_transform.hpp"
#include "../libs/glm/gtc/type_ptr.hpp"
#include "Terrabumper.h"


GLFWwindow* mainWindow = NULL;
int mainWindowHeight = 1080;
int mainWindowWidth = 1920;

GLuint shaderProgram = 0;
const int SHADER = 0;
const int PROGRAM = 1;
std::map<std::string, int> uniformArrayLocations;
bool bWireFrameMode = false;

const std::string bumpMap = "textures/Mega_Bump.tga";
const std::string testMap = "textures/checker16.tga";
GLuint vbo, vao, ibo;

float camYaw = 0.f;
float camPitch = 0.f;
float camRadius = 10.f;
float yawRad;
float pitchRad;

glm::vec3 camPosition = glm::vec3(0.f, 0.f, 0.f);
glm::vec3 camUp = glm::vec3(0.f, 1.f, 0.f);
glm::vec3 subjectPos = glm::vec3(0.f, 0.f, -50.f);

bool Initialize();
void LoadBuffer(Terrabumper landscape, GLuint& vbo, GLuint& vao, GLuint& ibo);
void CompileShaders(const char* vShader, const char* fShader);
std::string FileToString(const std::string& filename);
void ShaderCompilationCheck(unsigned int shader, int type);
void SetUniform(const char* name, glm::mat4 &matrix);
void MoveCamera(float dYaw, float dPitch);
void OnFrameBufferSize(GLFWwindow* window, int width, int height);
void OnKeyDown(GLFWwindow* window, int key, int scancode, int action, int mode);
void OnMouseMove(GLFWwindow* window, double posX, double posY);

GLfloat Quad(int i);			// DEBUGGING
GLuint QuadIndices(int i);		// DEBUGGING


int main()
{
	if (!Initialize())
		return -1;

	Terrabumper terrabumper;
	if (!terrabumper.LoadTGA(bumpMap))
		return -2;
	
	LoadBuffer(terrabumper, vbo, vao, ibo);
	CompileShaders("source/basicOrbCam.vert", "source/fixedColor.frag");

	while (!glfwWindowShouldClose(mainWindow))
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glm::mat4 model(1.f), view(1.f), projection(1.f);
		MoveCamera(camYaw, camPitch);
		model = glm::translate(model, subjectPos);
		view = glm::lookAt(camPosition, subjectPos, camUp);
		projection = glm::perspective(glm::radians(45.f), (float)mainWindowWidth / (float)mainWindowHeight, 0.1f, 100.f);
		
		glUseProgram(shaderProgram);
		SetUniform("model", model);
		SetUniform("view", view);
		SetUniform("projection", projection);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, terrabumper.vertexIndex.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glfwSwapBuffers(mainWindow);
		glfwPollEvents();
	}

	glDeleteProgram(shaderProgram);
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

	glClearColor(0.05f, 0.f, 0.f, 1.f);
	glViewport(0, 0, mainWindowWidth, mainWindowHeight);
	//glEnable(GL_DEPTH_TEST);

	return true;
}

void LoadBuffer(Terrabumper landscape, GLuint& vbo, GLuint& vao, GLuint& ibo)
{
	//for (int i = 0; i < 12; i++)
	//	landVertices.push_back(Quad(i));			// DEBUGGING
	//for (int i = 0; i < 6; i++)
	//	landIndex.push_back(QuadIndices(i));		// DEBUGGING

	GLfloat* data = &landscape.vertices[0];
	GLuint* indexData = &landscape.vertexIndex[0];

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, landscape.vertices.size() * sizeof(GLfloat), data, GL_STATIC_DRAW);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, landscape.vertexIndex.size() * sizeof(GLuint), indexData, GL_STATIC_DRAW);
}

void CompileShaders(const char* vShader, const char* fShader)
{
	std::string vsString = FileToString(vShader);
	std::string fsString = FileToString(fShader);
	const char* vsSourcePtr = vsString.c_str();
	const char* fsSourcePtr = fsString.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vs, 1, &vsSourcePtr, NULL);
	glShaderSource(fs, 1, &fsSourcePtr, NULL);
	glCompileShader(vs);
	ShaderCompilationCheck(vs, SHADER);
	glCompileShader(fs);
	ShaderCompilationCheck(fs, SHADER);
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vs);
	glAttachShader(shaderProgram, fs);
	glLinkProgram(shaderProgram);
	ShaderCompilationCheck(shaderProgram, PROGRAM);
	glDeleteShader(vs);
	glDeleteShader(fs);
}

std::string FileToString(const std::string &filename)
{
	std::stringstream ss;
	std::ifstream file;

	try
	{
		file.open(filename, std::ios::in);
		if (!file.fail())
		{
			ss << file.rdbuf();
		}

		file.close();
	}
	catch (std::exception err)
	{
		std::cerr << "Error reading shader filename." << std::endl;
	}

	return ss.str();
}

void ShaderCompilationCheck(unsigned int shader, int type)
{
	int status = 0;
	switch (type)
	{
		case SHADER:
			glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
			if (status == GL_FALSE)
			{
				int length = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
				std::string errorlog(length, ' ');
				glGetShaderInfoLog(shader, length, &length, &errorlog[0]);
				std::cout << "Shader failed to compile. " << errorlog << std::endl;
			}
			break;
		case PROGRAM:
			glGetProgramiv(shader, GL_LINK_STATUS, &status);
			if (status == GL_FALSE)
			{
				int length = 0;
				glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &length);
				std::string errorlog(length, ' ');
				glGetProgramInfoLog(shader, length, &length, &errorlog[0]);
				std::cout << "Shader Program Linker failure. " << errorlog << std::endl;
			}
			break;
		default:
			std::cout << "Shader Compilation Check misuse." << std::endl;
			break;
	}
}

void SetUniform(const char* name, glm::mat4 &matrix)
{
	int element = -1;
	for (const auto pair : uniformArrayLocations)
	{
		if (pair.first == name)
			element = pair.second;
	}
	if (element < 0)
	{
		element = glGetUniformLocation(shaderProgram, name);
		uniformArrayLocations[name] = element;
	}

	glUniformMatrix4fv(element, 1, GL_FALSE, glm::value_ptr(matrix));
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

void OnKeyDown(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		bWireFrameMode = !bWireFrameMode;
		if (bWireFrameMode)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
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

// The functions below are for debugging purposes only!
GLfloat Quad(int i)
{
	static GLfloat a[] = {
		//  X       Y       Z
			-0.5f,   0.5f,   0.f,
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