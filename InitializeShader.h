#pragma once
#include "GL\freeglut.h"

class InitializeShader
{
public:
	InitializeShader();
	~InitializeShader();
	GLuint AllocateBuffer(GLenum bufferType, const void* pData, size_t size);
	void linkProgram(GLuint program);
	// Helper function to load a text file
	char* readShaderSource(const char* shaderFile);

	GLuint InitShader(const char * vShaderFile, 
		const char * gShaderFile, 
		const char * fShaderFile, 
		const char * outputAttributeName);

	//  Helper function to load vertex and fragment shader files
	GLuint InitShader(const char* vertexShaderFile,
		const char* fragmentShaderFile,
		const char* outputAttributeName);

	GLuint InitComputeShader(const char * cShaderFile);

};

