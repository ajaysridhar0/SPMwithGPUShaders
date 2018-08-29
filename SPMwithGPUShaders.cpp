
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
MapData* mapReader = new MapData("simple2");

vec2 domainData[4] =
{
	vec2(-1,-1),
	vec2(-1,1),
	vec2(1,1),
	vec2(1,-1)
};

int PIXEL_AREA = 1024*1024;
float windowRatio = (mapReader->getInitDomain().x) / (mapReader->getInitDomain().y);
unsigned int WINDOW_HEIGHT = int(sqrt(PIXEL_AREA /windowRatio));
unsigned int WINDOW_WIDTH = int(PIXEL_AREA /WINDOW_HEIGHT);
// for reading the stencil value in a shader
unsigned int * stencilData; 

GLuint shaderProgram;
GLuint shadowAreaShaderProgram, coneShaderProgram;
GLuint searchComputeShader, distanceComputeShader;
GLuint dataArraySSbo;

// declaration of uniform variables
GLuint projectionUniform[3],
modelViewUniform[3],
colorUniform,
csvfUniform,
prevRenderUniform, 
stencilValuesUniform,
windowSizeUniform;

bool computeOnce = true;

// global uniform modelviewproject matrix values
mat4 projection = Ortho2D(-1., 1., -1., 1.);
mat4 modelView = Angel::Scale(1, 1, 1);

// declaration of Attribute variables
GLuint positionAttribute,
colorAttribute,
vertexAttribute[2];
//frame buffer
GLuint frameBufferObject[2];
//prev render
GLuint prevRender[2];
// two VAO for each shader process
GLuint vertexArrayObject[3];
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

void InitSPMSystem();
// buffer data
void loadDefaultBufferData();
void loadBufferData(const Vertex * vertices);

int pixelIndex(vec2 mapCoord)
{
	int x = int(((WINDOW_WIDTH -1)/ 2) * (mapCoord.x + 1));
	int y =	int(((WINDOW_HEIGHT -1) / 2) * (mapCoord.y + 1));
	return y * WINDOW_WIDTH + x;
}

void printStencilAdj(vec2 mapCoord)
{
	int shadowCounter = 0;
	int miniShadowCounter = 0;
	for (int i = 0; i <= 4; i++)
	{
		for (int j = 0; j <= 4; j ++)
		{
			cout << stencilData[pixelIndex(mapCoord) + (2-i)*WINDOW_WIDTH + j-2];
			if (stencilData[pixelIndex(mapCoord) + (2 - i)*WINDOW_WIDTH + j - 2] == 1)
			{
				shadowCounter++;
				if (i >= 1 && i <= 3 && j >= 1 && j <=3)
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
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	// disable blue component
	// glColorMask(GL_TRUE, GL_TRUE, GL_FALSE, GL_TRUE);
	glDepthMask(GL_TRUE);
	// dont draw the polygons in the stencil buffer
	glStencilMask(0x00);
	//-------------------------------
}

void generateMap()
{
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilMask(0x00);
	glDisable(GL_STENCIL_TEST);
	glUseProgram(shaderProgram);
	glBindVertexArray(vertexArrayObject[0]);
	// projection transformations with projection matrix
	glUniformMatrix4fv(projectionUniform[0], 1, GL_TRUE, projection);
	glUniformMatrix4fv(modelViewUniform[0], 1, GL_TRUE, modelView);
	glUniform3fv(colorUniform, 1, vec3(1., 1., 1.));
	for (int i = 0; i < mapReader->getNumOfPolygons(); i++)
	{
		// warning: works only for concave polygons
		//glDrawElements(GL_TRIANGLE_FAN, mapReader->getVertexData()[i + 1].size(), GL_UNSIGNED_INT, mapReader->getIndices()[i]);
	
	}
	glBindVertexArray(0);
	glUseProgram(0);
}


//stencil value = 1 in the shadow area
void generateShadowAreas()
{
	//from stack overflow--------------------------------------------------
	//// no fragments drawn
	//glStencilFunc(GL_NEVER, 1, 0xFF);
	//// disable writing to the depth buffer and the color buffer
	//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	//glDepthMask(GL_FALSE);
	//// enable writting to the stencil buffer
	//glStencilMask(0xFF);
	//glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
	//---------------------------------------------------------------------
	
	//from 1st paper-------------------------------------------------------
	/*glClear(GL_STENCIL_BUFFER_BIT);
	glDrawBuffer(GL_NONE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDisable(GL_CULL_FACE);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);*/
	//----------------------------------------------------------------------

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
	glBindVertexArray(vertexArrayObject[1]);
	glUniformMatrix4fv(projectionUniform[1], 1, GL_TRUE, projection);
	glUniformMatrix4fv(modelViewUniform[1], 1, GL_TRUE, modelView);
	glUniform1f(csvfUniform, GLfloat(csvf+1.));
	for (int i = 0; i < mapReader->getNumOfPolygons(); i++)
	{
		glDrawElements(GL_LINE_LOOP, mapReader->getVertexData()[i + 1].size(), GL_UNSIGNED_INT, mapReader->getIndices()[i]);
	}
	glBindVertexArray(0);
	glUseProgram(0);
	/*glEnable(GL_CULL_FACE);
	glDrawBuffer(GL_BACK);*/
}

void generateCones()
{
	// from 1st paper ---------------------------------------
	/*glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_EQUAL, 0, 0xFF);
	glBindVertexArray(0);
	glUseProgram(0);*/
	//--------------------------------------------------------

	//from wiki-------------------------------------------------
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilMask(0x00);
	// draw where stencil's value is 0
	glStencilFunc(GL_EQUAL, 0, 0xFF);
	//-----------------------------------------------------------

	//glGenFramebuffers(1, &frameBufferObject[0]);
	//glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject[0]);

	//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	//	cout << "Framebuffer succesfully binded" << endl;
	//else
	//{
	//	cerr << "ERROR::FRAMEBUFFER:: Framebuffer did not bind" << endl;
	//	return;
	//}

	//glGenTextures(1, &prevRender[0]);
	//glBindTexture(GL_TEXTURE_2D, prevRender[0]);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glBindTexture(GL_TEXTURE_2D, 0);

	//// attach the texture to the framebuffer
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, prevRender[0], 0);
	//
	//glUseProgram(coneShaderProgram);
	//glBindVertexArray(vertexArrayObject[1]);
	//glUniformMatrix4fv(projectionUniform[1], 1, GL_TRUE, projection);
	//glUniformMatrix4fv(modelViewUniform[1], 1, GL_TRUE, modelView);
	//glUniform1i(prevRenderUniform, (GLuint)prevRender);
	////draw rect that covers whole map
	//GLubyte mapDrawingOrder[4] = { 0, 1, 2, 3 };
	//glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, mapDrawingOrder);

	glUseProgram(shaderProgram);
	glBindVertexArray(vertexArrayObject[2]);
	// projection transformations with projection matrix
	glUniformMatrix4fv(projectionUniform[0], 1, GL_TRUE, projection);
	glUniformMatrix4fv(modelViewUniform[0], 1, GL_TRUE, modelView);
	glUniform3fv(colorUniform, 1, vec3(0.,  .5, .0));
	GLuint mapDrawingOrder[4] = { 0, 1, 2, 3 };
	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, mapDrawingOrder);
	glBindVertexArray(0);
	glUseProgram(0);

	//reads and stores the stencil values--------------------------------------------------------------------
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	stencilData = new unsigned int[WINDOW_WIDTH * WINDOW_HEIGHT];
	glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_STENCIL_INDEX, GL_UNSIGNED_INT, stencilData);
	if (true)
	{
		cout << "Pixel count:  " << WINDOW_WIDTH * WINDOW_HEIGHT << endl;
		printStencilAdj(vec2(-.75, .75));
		printStencilAdj(vec2(-.75, .25));
		printStencilAdj(vec2(-.25, .25));
		
		printStencilAdj(vec2(-.6, -.2));
		printStencilAdj(vec2(-.6, -.6));
		printStencilAdj(vec2(-.1, -.6));
		printStencilAdj(vec2(.2, -.4));
		printStencilAdj(vec2(.9, -.4));
		cout << "# of FULL 5x5 shadows: " << fullShadowCounter << endl;
		cout << "# of FULL 3x3 shadows: " << miniFullShadowCounter << endl;
		for(int i = 0; i < 35; i++)
		{
			cout << endl;
		}
		computeOnce = false;
	}
	//-------------------------------------------------------------------------------------------------------
	glDisable(GL_STENCIL_TEST);
}

void searchCompute()
{
	glUseProgram(searchComputeShader);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
	glUseProgram(0);
}

void distanceCompute()
{
	glUseProgram(distanceComputeShader);
	/*glGenFramebuffers(1, &frameBufferObject[1]);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject[1]);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		cout << "Framebuffer succesfully binded" << endl;
	else
	{
		cerr << "ERROR::FRAMEBUFFER:: Framebuffer did not bind" << endl;
		return;
	}
	
	glGenTextures(1, &prevRender[1]);
	glBindTexture(GL_TEXTURE_2D, prevRender[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// attach the texture to the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, prevRender[0], 0);*/
	GLuint windowSize[2] = {WINDOW_WIDTH, WINDOW_HEIGHT};
	glUniform1uiv(windowSizeUniform, 2, windowSize);
	glUniform1uiv(stencilValuesUniform, WINDOW_HEIGHT * WINDOW_WIDTH, stencilData);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
	glUseProgram(0);

}

void runSPM()
{	
	searchCompute();
	generateShadowAreas();
	generateCones();
}

void display()
{
	runSPM();
	generateMap();
	glutSwapBuffers();
	Angel::CheckError();
}

void reshape(int W, int H)
{
	WINDOW_WIDTH = W;
	WINDOW_HEIGHT = H;
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
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
	// most likely will change
	shaderProgram = init->InitShader("Shaders/Default-Vert.glsl", "Shaders/Default-Frag.glsl",
		"fFragColor");

	positionAttribute = glGetAttribLocation(shaderProgram, "position");
	if (positionAttribute == GL_INVALID_INDEX) {
		cerr << "Default shader did not contain the 'position' uniform." << endl;
	}
	// loading in all uniform variables to shaders
	projectionUniform[0] = glGetUniformLocation(shaderProgram, "projection");
	if (projectionUniform[0] == GL_INVALID_INDEX) {
		cerr << "Default shader did not contain the 'projection' uniform." << endl;
	}
	modelViewUniform[0] = glGetUniformLocation(shaderProgram, "modelView");
	if (modelViewUniform[0] == GL_INVALID_INDEX) {
		cerr << "Default shader did not contain the 'modelView' uniform." << endl;
	}
	colorUniform = glGetUniformLocation(shaderProgram, "color");
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
	stencilValuesUniform = glGetUniformLocation(distanceComputeShader, "stencilValues");
	if (csvfUniform == GL_INVALID_INDEX) {
		cerr << "Distance compute shader did not contain the 'stencilValues' uniform." << endl;
	}
	windowSizeUniform = glGetUniformLocation(distanceComputeShader, "windowSize");
	if (windowSizeUniform == GL_INVALID_INDEX) {
		cerr << "Distance compute shader did not contain the 'windowSize' uniform." << endl;
	}
}

void loadShadowAreaShader()
{
	shadowAreaShaderProgram = init->InitShader(
		"Shaders/ShadowArea-Vert.glsl",
		"Shaders/ShadowArea-Geom.glsl",
		"Shaders/ShadowArea-Frag.glsl",
		"fFragColor");
	// loading in all attribute variables to shaders
	vertexAttribute[0] = glGetAttribLocation(shadowAreaShaderProgram, "vertex");
	if (vertexAttribute[0] == GL_INVALID_INDEX) {
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
		"Shaders/Cone-Geom.glsl",
		"fFragColor");
	// loading in all attribute variables to shaders
	vertexAttribute[1] = glGetAttribLocation(coneShaderProgram, "vertex");
	if (vertexAttribute[1] == GL_INVALID_INDEX) {
		cerr << "Cone shader did not contain the 'vertex' attribute." << endl;
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
	prevRenderUniform = glGetUniformLocation(coneShaderProgram, "prevRender");
	if (prevRenderUniform == GL_INVALID_INDEX) {
		cerr << "Cone shader did not contain the 'prevRender' uniform." << endl;
	}
}

void InitSPMSystem()
{
	Vertex* dataArray = new Vertex[mapReader->getSizeOfDataArray()];
	Vertex* pDataArray = dataArray;
	pDataArray = mapReader->getDataArray();

	mapReader->printDataArray(pDataArray);
	loadBufferData(pDataArray);
	
	delete[] dataArray;
}

void loadDefaultBufferData()
{
	glGenVertexArrays(1, &vertexArrayObject[0]);
	glBindVertexArray(vertexArrayObject[0]);
	GLuint  vertexBuffer[2];
	glGenBuffers(1, &vertexBuffer[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, mapReader->getSizeOfDataArray() * sizeof(Vertex), mapReader->getDataArray(), GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(positionAttribute);
	// the source points are excluded from this vertex data
	glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), (const GLvoid *)((mapReader->getVertexData()[0].size() + 1) * sizeof(Vertex)));
	
	glGenBuffers(1, &vertexBuffer[1]);
	glGenVertexArrays(1, &vertexArrayObject[2]);
	glBindVertexArray(vertexArrayObject[2]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(vec2), &domainData, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (const GLvoid *)0);
}



void loadBufferData(const Vertex* dataArray)
{
	
	dataArraySSbo = init->AllocateBuffer(GL_SHADER_STORAGE_BUFFER, (float*)dataArray, (mapReader->getSizeOfDataArray()) * sizeof(Vertex));
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, dataArraySSbo);

	glGenVertexArrays(1, &vertexArrayObject[1]);
	glBindVertexArray(vertexArrayObject[1]);
	
	glBindBuffer(GL_ARRAY_BUFFER, dataArraySSbo);

	glEnableVertexAttribArray(0);
	// the source points are excluded from this vertex data
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), (const GLvoid *)((mapReader->getVertexData()[0].size()+1) * sizeof(Vertex)));
	glBindVertexArray(0);
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
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
	// glutIdleFunc(runSPM);
	// initializes OpenGL like init()
	Angel::InitOpenGL();
	mapReader->printData();
	mapReader->printDataArray(mapReader->getDataArray());
	mapReader->printIndices();
	loadDefaultShader();
	loadShadowAreaShader();
	loadSearchComputeShader();
	loadDistanceComputeShader();
	InitSPMSystem();
	/*
	loadConeShader();
	*/
	loadDefaultBufferData();
	Angel::CheckError();

	// enters the GLUT event processing loop
	glutMainLoop();
	delete mapReader;
	delete init;
}