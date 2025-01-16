#version 330 core

layout (location = 0) in vec3 pos;

// uniform vec2 offset;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vert_color;

void main()
{
   vert_color = vec3(0.6f, 0.6f, 0.2f);
   gl_Position = projection * view * model * vec4(pos, 1.f);
};