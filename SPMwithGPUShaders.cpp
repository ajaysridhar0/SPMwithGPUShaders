
/**
ShortestPathMaps.cpp
Purpose: Generates a shortest path map

@author Ajay Sridhar
@version 1.2 8/19/18
*/
#include "stdafx.h"
#include <iostream>
#include <string>
#include "Angel\Angel.h"
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "InitializeShader.h"
#include "MapData.h"

using namespace std;
using namespace Angel;

InitializeShader *init = new InitializeShader();
MapData* mapReader = new MapData("spiral");

vec2 domainData[8] =
{
	vec2(-1,-1), vec2(0.,0.),
	vec2(-1,1), vec2(0.,1.),
	vec2(1,1), vec2(1.,1.),
	vec2(1,-1), vec2(1.,0.)
};
GLuint mapDrawingOrder[4] = { 0, 1, 2, 3 };

unsigned int windowHeight = 1024;
unsigned int windowWidth = windowHeight;
// for reading the stencil value in a shader
unsigned int * stencilData;

GLuint defaultShaderProgram, drawSPMShaderProgram;
GLuint shadowAreaShaderProgram, coneShaderProgram;
GLuint searchComputeShader, distanceComputeShader;
GLuint dataArraySSbo, stencilValuesSSbo;

// declaration of uniform variables
GLuint projectionUniform[4],
modelViewUniform[4],
colorUniform,
csvfUniform,
prevRenderUniform[2];

int counter = 0;
bool computeOnce = true;

// global uniform modelviewproject matrix values
mat4 projection = Ortho2D(-1., 1., -1., 1.);
mat4 modelView = Angel::Scale(1, 1, 1);

// declaration of Attribute variables
GLuint
colorAttribute,
vertexAttribute[4],
texCoordAttribute[2];
//frame buffer
GLuint framebufferObject[2];
//prev render
GLuint prevRender[2];
// 2 VAO for each shader process
GLuint vertexArrayObject[2];
// compute shaders stuff
int fullShadowCounter = 0;
int miniFullShadowCounter = 0;
float csvf = 4.;

// forward declaration
int pixelIndex(vec2 mapCoord);
void printStencilAdj(vec2 mapCoord);
void globalState(); // done
void searchCompute(); // needs to be integrated
void generateMap(); // done
void generateShadowAreas(); // needs to be tested
void generateCones(); // work in progress
void distanceCompute(); // needs to be integrated
void drawSPM();
void runSPM();
// GLUT stuff
void display();
void reshape(int W, int H);
void keyboard(unsigned char key, int x, int y);
// default shader 
void loadDefaultShader();
// compute shader 
void loadDistanceComputeShader();
void loadSearchComputeShader();
// SPM shaders
void loadShadowAreaShader();
void loadConeShader();
void loadDrawSPMShader();

void InitSPMSystem();
// buffer data
void loadBufferData();
void enableUserFramebuffer();

int pixelIndex(vec2 mapCoord)
{
	int x = (int)(((windowWidth - 1) / 2) * (mapCoord.x + 1));
	int y = (int)(((windowHeight - 1) / 2) * (mapCoord.y + 1));
	return y * windowWidth + x;
}

void printStencilAdj(vec2 mapCoord)
{
	int shadowCounter = 0;
	int miniShadowCounter = 0;
	for (int i = 0; i <= 4; i++)
	{
		for (int j = 0; j <= 4; j++)
		{
			cout << stencilData[pixelIndex(mapCoord) + (2 - i)*windowWidth + j - 2];
			if (stencilData[pixelIndex(mapCoord) + (2 - i)*windowWidth + j - 2] == 1)
			{
				shadowCounter++;
				if (i >= 1 && i <= 3 && j >= 1 && j <= 3)
				{
					miniShadowCounter++;
				}
			}
		}
		cout << endl;
	}
	if (shadowCounter == 25)
	{
		fullShadowCounter++;
	}
	if (miniShadowCounter == 9)
	{
		miniFullShadowCounter++;
	}
	cout << endl;
}

void globalState()
{
	// configure global opengl state
	// -----------------------------
	glClearColor(1., 1., 1., 1.);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void generateMap()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilMask(0x00);
	glDisable(GL_STENCIL_TEST);
	glUseProgram(defaultShaderProgram);
	glBindVertexArray(vertexArrayObject[0]);
	// projection transformations with projection matrix
	glUniformMatrix4fv(projectionUniform[0], 1, GL_TRUE, projection);
	glUniformMatrix4fv(modelViewUniform[0], 1, GL_TRUE, modelView);
	glUniform3fv(colorUniform, 1, vec3(1., 1., 1.));
	for (int i = 0; i < mapReader->getNumOfPolygons(); i++)
	{
		// warning: works only for concave polygons
		glDrawElements(GL_LINE_LOOP, mapReader->getVertexData()[i + 1].size(), GL_UNSIGNED_INT, mapReader->getIndices()[i]);

	}
	glBindVertexArray(0);
	glUseProgram(0);
}

void searchCompute()
{
	glUseProgram(searchComputeShader);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
	glUseProgram(0);
}

//stencil value = 1 in the shadow area
void generateShadowAreas()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// glViewport(0, 0, windowWidth, windowHeight);
	//From OpenGL Wiki----------------------------------------------------------------
	/* onDisplay */
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	glStencilFunc(GL_NEVER, 1, 0xFF);
	glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);  // draw 1s on test fail (always)
												// draw stencil pattern
	glStencilMask(0xFF);
	glClear(GL_STENCIL_BUFFER_BIT);  // needs mask=0xFF
									 //--------------------------------------------------------------------------------

									 // draw the Shadow Areas to the stencil buffer
	glUseProgram(shadowAreaShaderProgram);
	glBindVertexArray(vertexArrayObject[0]);
	glUniformMatrix4fv(projectionUniform[1], 1, GL_TRUE, projection);
	glUniformMatrix4fv(modelViewUniform[1], 1, GL_TRUE, modelView);
	glUniform1f(csvfUniform, GLfloat(csvf+10));
	for (int i = 0; i < mapReader->getNumOfPolygons(); i++)
	{
		glDrawElements(GL_LINE_LOOP, mapReader->getVertexData()[i + 1].size(), GL_UNSIGNED_INT, mapReader->getIndices()[i]);
	}
	glBindVertexArray(0);
	glUseProgram(0);

	//reads and stores the stencil values--------------------------------------------------------------------
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	stencilData = new unsigned int[windowWidth * windowHeight];
	glReadPixels(0, 0, windowWidth, windowHeight, GL_STENCIL_INDEX, GL_UNSIGNED_INT, stencilData);
	// makes a ssbo for stencil values to read in Distance Compute Shader
	stencilValuesSSbo = init->AllocateBuffer(GL_SHADER_STORAGE_BUFFER, (unsigned int*)stencilData, windowHeight*windowWidth * sizeof(unsigned int));
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, stencilValuesSSbo);
	// print stencil values near vertices
	/*for (int i = mapReader->getVertexData()[0].size() + 1; i < mapReader->getSizeOfDataArray()+1; i++ )
	{
		cout << "Vertex: " << mapReader->getDataArray()[i].x << "," << mapReader->getDataArray()[i].y << ":" << endl;
		printStencilAdj(vec2(mapReader->getDataArray()[i].x, mapReader->getDataArray()[i].y));
		for (int i = 0; i < 3; i++)
		{
			cout << endl;
		}
	}
	cout << "# of FULL 5x5 shadows: " << fullShadowCounter << endl;
	cout << "# of FULL 3x3 shadows: " << miniFullShadowCounter << endl;
	for (int i = 0; i < 25; i++)
	{
		cout << endl;
	}*/
	cout << "iteration: " << counter << endl;
	counter++;
	//-------------------------------------------------------------------------------------------------------
}

void generateCones()
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferObject[0]);
	glViewport(0, 0, windowWidth, windowHeight);
	//glClearColor(0., 0., 0., 1.);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilMask(0x00);
	glDisable(GL_STENCIL_TEST);
	glUseProgram(coneShaderProgram);
	glBindVertexArray(vertexArrayObject[1]);
	// projection transformations with projection matrix
	glUniformMatrix4fv(projectionUniform[2], 1, GL_TRUE, projection);
	glUniformMatrix4fv(modelViewUniform[2], 1, GL_TRUE, modelView);
	glUniform1i(prevRenderUniform[0], (GLuint)0);
	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, mapDrawingOrder);
	glBindVertexArray(0);
	glUseProgram(0);
}

void distanceCompute()
{
	glUseProgram(distanceComputeShader);
	//glDispatchCompute(mapReader->getSizeOfDataArray(), 1, 1);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
	glUseProgram(0);
}

void drawSPM()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
	glClearColor(0., 0., 0., 1.);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_TRUE, GL_TRUE, GL_FALSE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilMask(0x00);
	glDisable(GL_STENCIL_TEST);
	glUseProgram(drawSPMShaderProgram);
	glBindVertexArray(vertexArrayObject[1]);
	// projection transformations with projection matrix
	glUniformMatrix4fv(projectionUniform[3], 1, GL_TRUE, projection);
	glUniformMatrix4fv(modelViewUniform[3], 1, GL_TRUE, modelView);
	glUniform1i(prevRenderUniform[1], (GLuint)0);
	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, mapDrawingOrder);
	glBindVertexArray(0);
	glUseProgram(0);
}

void runSPM()
{
	for (int i = 0; i < mapReader->getSizeOfDataArray() - 1; i++)
	{
		searchCompute();
		generateShadowAreas();
		generateCones();
		distanceCompute();
	}
}

void display()
{
	globalState();
	if (computeOnce)
	{
		runSPM();
		computeOnce = false;
	}
	drawSPM();
	generateMap();
	glutSwapBuffers();
	Angel::CheckError();
}

void reshape(int W, int H)
{
	windowWidth = W;
	windowHeight = H;
	glViewport(0, 0, windowWidth, windowHeight);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':
		csvf -= 0.4;
		glutPostRedisplay();
		break;
	case 'e':
		csvf += 0.4;
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

void loadDefaultShader()
{
	defaultShaderProgram = init->InitShader("Shaders/Default-Vert.glsl",
		"Shaders/Default-Frag.glsl",
		"fFragColor");

	vertexAttribute[0] = glGetAttribLocation(defaultShaderProgram, "vertex");
	if (vertexAttribute[0] == GL_INVALID_INDEX) {
		cerr << "Default shader did not contain the 'vertex' uniform." << endl;
	}
	// loading in all uniform variables to shaders
	projectionUniform[0] = glGetUniformLocation(defaultShaderProgram, "projection");
	if (projectionUniform[0] == GL_INVALID_INDEX) {
		cerr << "Default shader did not contain the 'projection' uniform." << endl;
	}
	modelViewUniform[0] = glGetUniformLocation(defaultShaderProgram, "modelView");
	if (modelViewUniform[0] == GL_INVALID_INDEX) {
		cerr << "Default shader did not contain the 'modelView' uniform." << endl;
	}
	colorUniform = glGetUniformLocation(defaultShaderProgram, "color");
	if (colorUniform == GL_INVALID_INDEX) {
		cerr << "Default shader did not contain the 'color' uniform." << endl;
	}
}

void loadSearchComputeShader()
{
	// Initialize and create the compute shader that sequentially search DataArray
	searchComputeShader = init->InitComputeShader("Shaders/SPM-SearchCompute.glsl");

}

void loadDistanceComputeShader()
{
	// Initialize and create the compute shader that update the siatances in the search DataArray
	distanceComputeShader = init->InitComputeShader("Shaders/SPM-DistanceCompute.glsl");
}

void loadShadowAreaShader()
{
	shadowAreaShaderProgram = init->InitShader(
		"Shaders/ShadowArea-Vert.glsl",
		"Shaders/ShadowArea-Geom.glsl",
		"Shaders/ShadowArea-Frag.glsl",
		"fFragColor");
	// loading in all attribute variables to shaders
	vertexAttribute[1] = glGetAttribLocation(shadowAreaShaderProgram, "vertex");
	if (vertexAttribute[1] == GL_INVALID_INDEX) {
		cerr << "Shadow area shader did not contain the 'vertex' attribute." << endl;
	}
	// loading in all uniform variables to shaders
	projectionUniform[1] = glGetUniformLocation(shadowAreaShaderProgram, "projection");
	if (projectionUniform[1] == GL_INVALID_INDEX) {
		cerr << "Shadow area shader did not contain the 'projection' uniform." << endl;
	}
	modelViewUniform[1] = glGetUniformLocation(shadowAreaShaderProgram, "modelView");
	if (modelViewUniform[1] == GL_INVALID_INDEX) {
		cerr << "Shadow area shader did not contain the 'modelView' uniform." << endl;
	}
	csvfUniform = glGetUniformLocation(shadowAreaShaderProgram, "csvf");
	if (csvfUniform == GL_INVALID_INDEX) {
		cerr << "Shadow area shader did not contain the 'csvf' uniform." << endl;
	}
}

void loadConeShader()
{
	coneShaderProgram = init->InitShader(
		"Shaders/Cone-Vert.glsl",
		"Shaders/Cone-Frag.glsl",
		"fFragColor");
	// loading in all attribute variables to shaders
	vertexAttribute[2] = glGetAttribLocation(coneShaderProgram, "vertex");
	if (vertexAttribute[2] == GL_INVALID_INDEX) {
		cerr << "Cone shader did not contain the 'vertex' attribute." << endl;
	}
	texCoordAttribute[0] = glGetAttribLocation(coneShaderProgram, "texCoord");
	if (texCoordAttribute[0] == GL_INVALID_INDEX) {
		cerr << "Cone shader did not contain the 'texCoord' attribute." << endl;
	}
	// loading in all uniform variables to shaders
	projectionUniform[2] = glGetUniformLocation(coneShaderProgram, "projection");
	if (projectionUniform[2] == GL_INVALID_INDEX) {
		cerr << "Cone shader did not contain the 'projection' uniform." << endl;
	}
	modelViewUniform[2] = glGetUniformLocation(coneShaderProgram, "modelView");
	if (modelViewUniform[2] == GL_INVALID_INDEX) {
		cerr << "Cone shader did not contain the 'modelView' uniform." << endl;
	}
	prevRenderUniform[0] = glGetUniformLocation(coneShaderProgram, "prevRender");
	if (prevRenderUniform[0] == GL_INVALID_INDEX) {
		cerr << "Cone shader did not contain the 'prevRender' uniform." << endl;
	}
}

void loadDrawSPMShader()
{
	drawSPMShaderProgram = init->InitShader(
		"Shaders/DrawSPM-Vert.glsl",
		"Shaders/DrawSPM-Frag.glsl",
		"fFragColor");
	// loading in all attribute variables to shaders
	vertexAttribute[3] = glGetAttribLocation(drawSPMShaderProgram, "vertex");
	if (vertexAttribute[3] == GL_INVALID_INDEX) {
		cerr << "DrawSPM shader did not contain the 'vertex' attribute." << endl;
	}
	texCoordAttribute[1] = glGetAttribLocation(drawSPMShaderProgram, "texCoord");
	if (texCoordAttribute[1] == GL_INVALID_INDEX) {
		cerr << "DrawSPM shader did not contain the 'texCoord' attribute." << endl;
	}
	// loading in all uniform variables to shaders
	projectionUniform[3] = glGetUniformLocation(drawSPMShaderProgram, "projection");
	if (projectionUniform[3] == GL_INVALID_INDEX) {
		cerr << "DrawSPM shader did not contain the 'projection' uniform." << endl;
	}
	modelViewUniform[3] = glGetUniformLocation(drawSPMShaderProgram, "modelView");
	if (modelViewUniform[3] == GL_INVALID_INDEX) {
		cerr << "DrawSPM shader did not contain the 'modelView' uniform." << endl;
	}
	prevRenderUniform[1] = glGetUniformLocation(drawSPMShaderProgram, "prevRender");
	if (prevRenderUniform[1] == GL_INVALID_INDEX) {
		cerr << "DrawSPM shader did not contain the 'prevRender' uniform." << endl;
	}
}

void InitSPMSystem()
{
	Vertex* dataArray = new Vertex[mapReader->getSizeOfDataArray()];
	Vertex* pDataArray = dataArray;
	pDataArray = mapReader->getDataArray();
	mapReader->printDataArray(pDataArray);
	dataArraySSbo = init->AllocateBuffer(GL_SHADER_STORAGE_BUFFER, (float*)pDataArray, (mapReader->getSizeOfDataArray()) * sizeof(Vertex));
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, dataArraySSbo);
	delete[] dataArray;
}

void loadBufferData()
{
	glGenVertexArrays(1, &vertexArrayObject[0]);
	glBindVertexArray(vertexArrayObject[0]);
	GLuint  vertexBuffer[2];
	glGenBuffers(1, &vertexBuffer[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, mapReader->getSizeOfDataArray() * sizeof(Vertex), mapReader->getDataArray(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(vertexAttribute[0]);
	// the source points are excluded from this vertex data
	glVertexAttribPointer(vertexAttribute[0], 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), (const GLvoid *)((mapReader->getVertexData()[0].size() + 1) * sizeof(Vertex)));
	glBindVertexArray(0);

	glGenBuffers(1, &vertexBuffer[1]);
	glGenVertexArrays(1, &vertexArrayObject[1]);
	glBindVertexArray(vertexArrayObject[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(vec2), &domainData, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(vec2), (const GLvoid *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(vec2), (const GLvoid *)(sizeof(vec2)));
	glBindVertexArray(0);
}

void enableUserFramebuffer()
{
	glGenFramebuffers(1, &framebufferObject[0]);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferObject[0]);

	/*	glGenTextures(1, &prevRender[0]);
	glBindTexture(GL_TEXTURE_2D, prevRender[0]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, windowWidth, windowHeight);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)*/;

	glGenTextures(1, &prevRender[0]);
	glBindTexture(GL_TEXTURE_2D, prevRender[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, prevRender[0], 0);


	//glGenTextures(1, &prevRender[1]);
	//glBindTexture(GL_TEXTURE_2D, prevRender[1]);
	//glTexStorage2D(GL_TEXTURE_2D, 1, GL_STENCIL_INDEX8, windowWidth, windowHeight);

	// attach the textures to the framebuffer
	// glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, prevRender[0], 0);
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, prevRender[1], 0);

	switch (glCheckFramebufferStatus(GL_FRAMEBUFFER))
	{
	case GL_FRAMEBUFFER_COMPLETE:
		cout << "Framebuffer succesfully binded" << endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		cout << "ERROR::FRAMEBUFFER::GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		cout << "ERROR::FRAMEBUFFER::GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT " << endl;
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		cout << "ERROR::FRAMEBUFFER::GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER " << endl;
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		cout << "ERROR::FRAMEBUFFER::GL_FRAMEBUFFER_UNSUPPORTED " << endl;
		break;
	default:
		cerr << "ERROR::FRAMEBUFFER:: Framebuffer did not bind" << endl;
		break;
	}
	// make sure we clear the framebuffer's content
	//glBindFramebuffer(GL_FRAMEBUFFER, framebufferObject[0]);
	glViewport(0, 0, windowWidth, windowHeight);
	glClearColor(0.0, 0.0, 1.0, 0.);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/*static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, draw_buffers);*/
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	// sets openGL version we use
	glutInitContextVersion(3, 2);
	// sets flags in GLUT Library (all caps bit fields)
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	// sets profile of GLUT libray
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutSetOption(
		GLUT_ACTION_ON_WINDOW_CLOSE,
		GLUT_ACTION_GLUTMAINLOOP_RETURNS
	);
	// Utilizes the core-profile to allow shaders
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL | GLUT_3_2_CORE_PROFILE);
	glutCreateWindow("SPM with GPU shaders");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutReshapeWindow(windowWidth, windowHeight);
	// glutIdleFunc(runSPM);
	// initializes OpenGL like init()
	Angel::InitOpenGL();
	mapReader->printData();
	mapReader->printDataArray(mapReader->getDataArray());
	mapReader->printIndices();
	loadDefaultShader();
	loadDrawSPMShader();
	loadSearchComputeShader();
	loadShadowAreaShader();
	loadDistanceComputeShader();
	loadConeShader();

	InitSPMSystem();
	loadBufferData();
	enableUserFramebuffer();
	Angel::CheckError();

	// enters the GLUT event processing loop
	glutMainLoop();
	delete mapReader;
	delete init;
}