#define GLEW_STATIC
#include "glew.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include "../libs/glm/gtc/matrix_transform.hpp"

const std::string filename = "../textures/Mega_Bump.tga";

int main()
{
	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	window = glfwCreateWindow(1080, 1920, "BumpLand - Bumpmap to Landscape", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -2;
	}

	glfwMakeContextCurrent(window);
	glClearColor(0.15f, 0.f, 0.f, 1.f);

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}