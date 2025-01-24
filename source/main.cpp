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
const std::string mainWindowTitle = "BumpLand - Bumpmap to Landscape";
int mainWindowHeight = 1080;
int mainWindowWidth = 1920;

GLuint shaderProgram = 0;
float invHS = 0.f;				// Assigned in LoadBuffer()
const int SHADER = 0;
const int PROGRAM = 1;
std::map<std::string, int> uniformArrayLocations;
bool bWireFrameMode = false;
std::string vertShader = "shaders/basicOrbCam.vert";
std::string fragShader = "shaders/heightShaded.frag";

std::string bumpMap = "textures/Mega_Bump.tga";
std::string testMap = "textures/checker32.tga";
GLuint vbo, vao, ibo;
int height = 1;

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
std::string ShaderFileToString(const std::string& filename);
void ShaderCompilationCheck(unsigned int shader, int type);
void SetUniform(const char* name, float &variable);
void SetUniform(const char* name, glm::mat4 &matrix);
void MoveCamera(float dYaw, float dPitch);
void ShowFramerate(GLFWwindow* window, double deltaTime, int triangles);
void OnFrameBufferSize(GLFWwindow* window, int width, int height);
void OnKeyDown(GLFWwindow* window, int key, int scancode, int action, int mode);
void OnMouseMove(GLFWwindow* window, double posX, double posY);
bool Instructions(int args, std::vector<std::string> arg);


int main(int argc, char* argv[])
{
	std::vector<std::string> a;
	for (int i = 0; i < argc; i++)
		a.push_back(argv[i]);

	if (!Instructions(argc, a))
		return 0;

	if (!Initialize())
		return -1;

	Terrabumper terrabumper;
	if (!terrabumper.LoadTGA(bumpMap, height))
		return -2;
	
	LoadBuffer(terrabumper, vbo, vao, ibo);
	CompileShaders(vertShader.c_str(), fragShader.c_str());
	
	double previousTime = glfwGetTime();

	while (!glfwWindowShouldClose(mainWindow))
	{
		double currentTime = glfwGetTime();
		double deltaTime = currentTime - previousTime;

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
		SetUniform("invHS", invHS);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, terrabumper.vertexIndex.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glfwSwapBuffers(mainWindow);
		glfwPollEvents();
		ShowFramerate(mainWindow, deltaTime, terrabumper.triangles);
		previousTime = currentTime;
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	if (!glfwInit())
		return false;

	mainWindow = glfwCreateWindow(mainWindowWidth, mainWindowHeight, mainWindowTitle.c_str(), NULL, NULL);
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
	glfwSwapInterval(0);
	//glEnable(GL_DEPTH_TEST);

	return true;
}

void LoadBuffer(Terrabumper landscape, GLuint& vbo, GLuint& vao, GLuint& ibo)
{
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
	invHS = (1.f / landscape.heightScale) * 0.0009f;
}

void CompileShaders(const char* vShader, const char* fShader)
{
	std::string vsString = ShaderFileToString(vShader);
	std::string fsString = ShaderFileToString(fShader);
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

std::string ShaderFileToString(const std::string &filename)
{
	std::stringstream ss;
	std::ifstream file;

	try
	{
		file.open(filename, std::ios::in);
		if (!file.fail())
			ss << file.rdbuf();

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

void SetUniform(const char* name, float &variable)
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

	glUniform1f(element, variable);
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

void ShowFramerate(GLFWwindow* window, double deltaTime, int triangles)
{
	static int frameCount = 0;
	static double elapsedSeconds = 0.0;

	elapsedSeconds += deltaTime;

	if (elapsedSeconds > 0.5)
	{
		double fps = (double)frameCount / elapsedSeconds;

		std::ostringstream outs;
		outs.precision(3);
		outs << std::fixed << mainWindowTitle << "  -  Triangles rendered: " << triangles << "  -  Current FPS: " << fps;
		glfwSetWindowTitle(window, outs.str().c_str());

		frameCount = 0;
		elapsedSeconds = 0.0;
	}

	frameCount++;
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

	if (key == GLFW_KEY_KP_ADD && action == GLFW_PRESS)
		invHS += 2.f;
	if (key == GLFW_KEY_KP_SUBTRACT && action == GLFW_PRESS)
		invHS -= 2.f;
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

bool Instructions(int args, std::vector<std::string> arg)
{
	if (args == 1)
	{
		std::cout << "Commands for loading:\n" <<
			"-fm followed by filename of bumpmap (only uncompressed .tga is supported currently).\n" <<
			"-tm sets file to be loaded to test map.\n" <<
			"-s sets viewer to single colour (default is shaded by height).\n" <<
			"-h followed by a number (1-100) sets height scaling of landscape to that number (default is 1).\n" <<
			"Example: bumpland -f textures/checker16.tga -h 10 -s\n\n" <<
			"Commands in viewer:\n" <<
			"Hold right mouse button and move mouse to zoom in/out.\n" <<
			"Hold left mouse button and move mouse to rotate object.\n" <<
			"Press 'W' to toggle wireframe viewing mode.\n" <<
			"If in heightshaded colour mode, press '+'/'-' on keypad to change brightness.\n" <<
			"Press 'esc' or close window with mouse to exit.\n\n" <<
			"Please be aware that large images demand a lot of memory and take a while to process.\n" <<
			"Example: 4096 x 4096 pixels image requires more than 600 mb and may take in excess of ten seconds to process.\n";
		return false;
	}

	int i = 1;

	while (i < args)
	{
		if (arg[i] == "-fm" && args >= i + 1)
		{
			bumpMap = arg[i + 1];
			i++;
		}
		else if (arg[i] == "-fm" && args < i + 1)
		{
			std::cout << "Please provide filename.\n";
			return false;
		}
		else if (arg[i] == "-s")
			fragShader = "shaders/fixedColor.frag";
		else if (arg[i] == "-tm")
			bumpMap = testMap;
		else if (arg[i] == "-h")
		{
			if (args == i)
				break;
			std::stringstream ss;
			ss << arg[i + 1];
			ss >> height;
			if (height < 1 || height > 100)
			{
				height = 1;
				std::cout << "-h number must be greater than 0. Numbers greater than 100 not advised.\n";
			}
			i++;
		}

		i++;
	}

	return true;
}
