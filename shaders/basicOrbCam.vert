#version 330 core

layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out float height;

void main()
{
   gl_Position = projection * view * model * vec4(pos, 1.f);
   height = pos.y;
};