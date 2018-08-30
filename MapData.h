#pragma once

#include <vector>
#include <string>
#include "Angel/Angel.h"

using namespace std;

// vertices in DataArray
struct Vertex {
	float x;
	float y;
	float distance;
	float status;
	// status > 0: source
	// status = 0: expanded
	// status < 0: obstacle
	float parentID;
	float ID;
};

class MapData
{
public:
	//MapData();
	MapData(string mapName);
	~MapData();
	void readSettings();
	void readData();
	void addSourcePoint(vec2 sourcePoint);
	void printData();
	void printIndices();
	void printDataArray(Vertex* dataArray);
	void setDataArray();
	//void setVertexIDs();
	/*TODO: change the return type to GLuint*: */
	void setIndices();
	vector<vec2>* getVertexData();
	Vertex * getDataArray();
	GLuint** getIndices();
	vec2 getInitDomain();
	int getSizeOfDataArray();
	int getNumOfPolygons();

private:
	float domain;
	string filename;
	string line;
	int numOfPolygons;
	int numOfVertices;
	vector<vec2> *vertexData;
	Vertex *dataArray;
	GLuint **indices;
};

