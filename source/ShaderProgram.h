#pragma once
#define GLEW_STATIC
#include "glew.h"
#include <string>
#include <map>
#include "../libs/glm/glm.hpp"


class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram();

	enum ShaderType
	{
		VERTEX,
		FRAGMENT,
		PROGRAM
	};

	bool LoadShaders(const char* vsFilename, const char* fsFilename);
	void Use();
	void SetUniform(const char* name, const glm::vec2& vector2D);
	void SetUniform(const char* name, const glm::vec3& vector3D);
	void SetUniform(const char* name, const glm::vec4& vector4D);
	void SetUniform(const char* name, const glm::mat4& matrix4D);
	GLuint GetProgram() const;

private:
	std::string FileToString(const std::string& filename);
	void CheckCompileErrors(unsigned int shader, ShaderType type);
	int GetUniformLocation(const char* name);

	unsigned int handle;
	std::map<std::string, int> uniformLocations;
};