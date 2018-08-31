//-----------------------------------------
// Cone-Vert.glsl
// the 3rd shader program in the algorithm 
//-------------------------------------------

#version 430 compatibility
#extension GL_ARB_shader_storage_buffer_object : require


layout (location = 0) in vec2 vertex; 
layout (location = 1) in vec2 texCoord; 
uniform mat4 projection, modelView;
out vec2 vTexCoord;

void main()
{
	gl_Position = projection * modelView * vec4(vertex.xy, 0., 1.);
	vTexCoord = texCoord;
} 