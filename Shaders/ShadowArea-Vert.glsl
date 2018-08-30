//ShadowArea-VertexShader.glsl
#version 420

in vec4 vertex;
// TODO: add this to attribute variables in SPMwithGPUShader.cpp


void main()
{
    gl_Position = vec4(vertex.xy, 0., 1.);
}