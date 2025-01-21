#version 330 core

in float height;
uniform float invHS;
out vec4 frag_color;

void main()
{
   float rx = height * invHS;
   float gx = height * invHS;
   float bx = height * invHS * 0.5f;
   frag_color = vec4(rx, gx, bx, 1.f);
};