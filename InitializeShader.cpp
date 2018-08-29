#include "stdafx.h"
#include <iostream>
#include <string>
#include "GL\glew.h"
#include "GL\freeglut.h"
#include "Angel\Angel.h"


#include "InitializeShader.h"


using namespace Angel;

InitializeShader::InitializeShader()
{
	
}


InitializeShader::~InitializeShader()
{
}

GLuint InitializeShader::AllocateBuffer(GLenum bufferType, const void * pData, size_t size)
{
	GLuint bufferObject = 0;

	//Generate a handle
	glGenBuffers(1, &bufferObject);

	//Bind
	glBindBuffer(bufferType, bufferObject);

	//Store data
	if (pData != nullptr)
		glBufferData(bufferType, size, pData, GL_STATIC_DRAW);

	//Unbind it
	glBindBuffer(bufferType, 0);
	return bufferObject;
}

void InitializeShader::linkProgram(GLuint program)
{
	/* link  and error check */
	glLinkProgram(program);
	GLint  linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked) {
		std::cerr << "Shader program failed to link" << std::endl;
		GLint  logSize;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
		char* logMsg = new char[logSize];
		glGetProgramInfoLog(program, logSize, NULL, logMsg);
		std::cerr << logMsg << std::endl;
		delete[] logMsg;

		exit(EXIT_FAILURE);
	}
}

// Create a NULL-terminated string by reading the provided file
char*
InitializeShader::readShaderSource(const char* shaderFile)
{
	FILE *filePointer;
	char *content = NULL;

	int count = 0;

	if (shaderFile != NULL) {
		filePointer = fopen(shaderFile, "rt");

		if (filePointer != NULL) {

			fseek(filePointer, 0, SEEK_END);
			count = ftell(filePointer);
			rewind(filePointer);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count + 1));
				count = fread(content, sizeof(char), count, filePointer);
				content[count] = '\0';
			}
			fclose(filePointer);
		}
	}
	return content;

}
// Create a GLSL program object from vertex, geometry, and fragment shader files
GLuint
InitializeShader::InitShader(const char* vShaderFile, const char* gShaderFile, const char* fShaderFile, const char* outputAttributeName) {
	struct Shader {
		const char*  filename;
		GLenum       type;
		GLchar*      source;
	}  shaders[3] = {
		{ vShaderFile, GL_VERTEX_SHADER, NULL },
		{ gShaderFile, GL_GEOMETRY_SHADER, NULL },
		{ fShaderFile, GL_FRAGMENT_SHADER, NULL }
	};

	GLuint program = glCreateProgram();

	for (int i = 0; i < 3; ++i) {
		Shader& s = shaders[i];
		// reads the shader source file
		s.source = readShaderSource(s.filename);
		// failure to read shader source file
		if (shaders[i].source == NULL) {
			std::cerr << "Failed to read " << s.filename << std::endl;
			exit(EXIT_FAILURE);
		}

		GLuint shader = glCreateShader(s.type);
		glShaderSource(shader, 1, (const GLchar**)&s.source, NULL);
		glCompileShader(shader);

		// check to see if compiled
		GLint  compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

		// if didn't comple
		if (!compiled) {
			std::cerr << s.filename << " failed to compile:" << std::endl;
			GLint  logSize;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
			char* logMsg = new char[logSize];
			glGetShaderInfoLog(shader, logSize, NULL, logMsg);
			std::cerr << logMsg << std::endl;
			delete[] logMsg;

			exit(EXIT_FAILURE);
		}

		delete[] s.source;

		glAttachShader(program, shader);
	}

	/* Link output variables */
	glBindFragDataLocation(program, 0, outputAttributeName);

	/* link  and error check */
	linkProgram(program);

	return program;
}

// Create a GLSL program object from vertex and fragment shader files
GLuint
InitializeShader::InitShader(const char* vShaderFile, const char* fShaderFile, const char* outputAttributeName) {
	struct Shader {
		const char*  filename;
		GLenum       type;
		GLchar*      source;
	}  shaders[2] = {
		{ vShaderFile, GL_VERTEX_SHADER, NULL },
		{ fShaderFile, GL_FRAGMENT_SHADER, NULL }
	};

	GLuint program = glCreateProgram();

	for (int i = 0; i < 2; ++i) {
		Shader& s = shaders[i];
		s.source = readShaderSource(s.filename);
		if (shaders[i].source == NULL) {
			std::cerr << "Failed to read " << s.filename << std::endl;
			exit(EXIT_FAILURE);
		}
		GLuint shader = glCreateShader(s.type);
		glShaderSource(shader, 1, (const GLchar**)&s.source, NULL);
		glCompileShader(shader);

		GLint  compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled) {
			std::cerr << s.filename << " failed to compile:" << std::endl;
			GLint  logSize;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
			char* logMsg = new char[logSize];
			glGetShaderInfoLog(shader, logSize, NULL, logMsg);
			std::cerr << logMsg << std::endl;
			delete[] logMsg;

			exit(EXIT_FAILURE);
		}

		delete[] s.source;

		glAttachShader(program, shader);
	}

	/* Link output */
	glBindFragDataLocation(program, 0, outputAttributeName);

	linkProgram(program);

	return program;
}

GLuint InitializeShader::InitComputeShader(const char* cShaderFile)
{
	struct Shader {
		const char*  filename;
		GLenum       type;
		GLchar*      source;
	}  shaders = { cShaderFile, GL_COMPUTE_SHADER, NULL };
	GLuint program = glCreateProgram();
	Shader& s = shaders;
	// reads the shader source file
	s.source = readShaderSource(s.filename);
	// failure to read shader source file
	if (shaders.source == NULL) {
		std::cerr << "Failed to read " << s.filename << std::endl;
		exit(EXIT_FAILURE);
	}

	GLuint shader = glCreateShader(s.type);
	glShaderSource(shader, 1, (const GLchar**)&s.source, NULL);
	glCompileShader(shader);

	// check to see if compiled
	GLint  compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	// if didn't comple
	if (!compiled) {
		std::cerr << s.filename << " failed to compile:" << std::endl;
		GLint  logSize;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
		char* logMsg = new char[logSize];
		glGetShaderInfoLog(shader, logSize, NULL, logMsg);
		std::cerr << logMsg << std::endl;
		delete[] logMsg;

		exit(EXIT_FAILURE);
	}

	delete[] s.source;

	glAttachShader(program, shader);

	/* link  and error check */
	linkProgram(program);

	/* use program object */
	glUseProgram(program);

	return program;
}
