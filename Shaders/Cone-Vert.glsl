//-----------------------------------------
// Cone-Vert.glsl
// the 3rd shader program in the algorithm 
/-------------------------------------------

#version 430 compatibility
#extension GL_ARB_shader_storage_buffer_object : require

layout(shared, binding = 0) buffer storage
{
    vec4 vertices[];
	// <x, y, dist, index>
}

 layout(shared, binding = 1) buffer storage
 {
     vec3 vertexIDs[];
     // <status, parentID>
 }

in layout (location = 0) vec4 aVertex; 
uniform mat4 projection, modelView;

void main()
{
	gl_Position = projection * modelView * vec4(aVertex.xy, 0. 1.);
} 