#version 150
//Default fragment shader

uniform vec3 color;
out vec4 fFragColor;

void main()
{
	fFragColor = vec4(color, 1.);
}