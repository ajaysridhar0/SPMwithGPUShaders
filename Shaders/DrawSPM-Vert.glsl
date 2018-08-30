#version 330 compatibility

uniform mat4 modelView;
uniform mat4 projection;

in vec2 vertex;


void main()
{
	gl_Position = modelView * projection * vec4(vertex.xy, 0., 1.);
}