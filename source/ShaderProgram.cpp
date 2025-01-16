#include "ShaderProgram.h"
#include "../libs/glm/gtc/type_ptr.hpp"
#include <fstream>
#include <iostream>
#include <sstream>


ShaderProgram::ShaderProgram()
{
    handle = 0;
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(handle);
}

bool ShaderProgram::LoadShaders(const char* vsFilename, const char* fsFilename)
{
    std::string vsString = FileToString(vsFilename);
    std::string fsString = FileToString(fsFilename);
    const char* vsSourcePtr = vsString.c_str();
    const char* fsSourcePtr = fsString.c_str();
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vs, 1, &vsSourcePtr, NULL);
    glShaderSource(fs, 1, &fsSourcePtr, NULL);
    glCompileShader(vs);
    CheckCompileErrors(vs, VERTEX);
    glCompileShader(fs);
    CheckCompileErrors(fs, FRAGMENT);

    handle = glCreateProgram();
    glAttachShader(handle, vs);
    glAttachShader(handle, fs);
    glLinkProgram(handle);
    CheckCompileErrors(handle, PROGRAM);

    glDeleteShader(vs);
    glDeleteShader(fs);
    uniformLocations.clear();

    return true;
}

void ShaderProgram::Use()
{
    if (handle > 0)
        glUseProgram(handle);
}

void ShaderProgram::SetUniform(const char* name, const glm::vec2& vector2D)
{
    int location = GetUniformLocation(name);
    glUniform2f(location, vector2D.x, vector2D.y);
}

void ShaderProgram::SetUniform(const char* name, const glm::vec3& vector3D)
{
    int location = GetUniformLocation(name);
    glUniform3f(location, vector3D.x, vector3D.y, vector3D.z);
}

void ShaderProgram::SetUniform(const char* name, const glm::vec4& vector4D)
{
    int location = GetUniformLocation(name);
    glUniform4f(location, vector4D.x, vector4D.y, vector4D.z, vector4D.w);
}

void ShaderProgram::SetUniform(const char* name, const glm::mat4& matrix4D)
{
    int location = GetUniformLocation(name);
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix4D));
}

GLuint ShaderProgram::GetProgram() const
{
    return handle;
}

std::string ShaderProgram::FileToString(const std::string& filename)
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


void ShaderProgram::CheckCompileErrors(unsigned int shader, ShaderType type)
{
    int status = 0;

    if (type == PROGRAM)
    {
        glGetProgramiv(handle, GL_LINK_STATUS, &status);
        if (status == GL_FALSE)
        {
            int length = 0;
            glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &length);
            std::string errorlog(length, ' ');
            glGetProgramInfoLog(handle, length, &length, &errorlog[0]);
            std::cout << "Shader Program Linker failure. " << errorlog << std::endl;
        }
    }
    else
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
        {
            int length = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            std::string errorlog(length, ' ');
            glGetShaderInfoLog(shader, length, &length, &errorlog[0]);
            std::cout << "Shader failed to compile. " << errorlog << std::endl;
        }
    }
}

int ShaderProgram::GetUniformLocation(const char* name)
{
    std::map<std::string, int>::iterator it = uniformLocations.find(name);
    if (it == uniformLocations.end())
        uniformLocations[name] = glGetUniformLocation(handle, name);

    return uniformLocations[name];
}