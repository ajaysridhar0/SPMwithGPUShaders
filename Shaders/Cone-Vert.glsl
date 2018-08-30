//-----------------------------------------
// Cone-Vert.glsl
// the 3rd shader program in the algorithm 
//-------------------------------------------

#version 430 compatibility
#extension GL_ARB_shader_storage_buffer_object : require


in vec4 vertex; 
uniform mat4 projection, modelView;

void main()
{
	gl_Position = projection * modelView * vec4(vertex.xy, 0., 1.);
} 