#version 150

uniform mat4 modelView;
uniform mat4 projection;

in vec2 position;


void main()
{
	gl_Position = modelView * projection * vec4(position.xy, 0., 1.);
}