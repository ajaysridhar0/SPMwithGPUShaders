#include "stdafx.h"
#include "MapData.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

ifstream file;
char endLine;


MapData::MapData(string mapName)
{
	numOfVertices = 0;
	filename = "Maps/" + mapName + ".txt";
	file.open(filename);

	readData();
	setDataArray();
	//setVertexIDs();
	setIndices();
}


MapData::~MapData()
{
	delete[] dataArray;
	for (int i = 0; i < numOfPolygons; i++)
	{
		delete[] indices[i];
		vertexData[i].clear();
	}
	delete[] vertexData;
	delete[] indices;

}


void MapData::readSettings()
{
	string checkNumPolygons;
	string checkDomain;
	//float bottom, left, top, right;
	float width, height;
	if (file.is_open())
	{
		file >> checkNumPolygons >> numOfPolygons >>
			checkDomain >> domain;
		
		//numOfPolygons++;
	}
	else
	{
		cerr << "ERROR: " << filename << " DID NOT OPEN!" << endl;
		exit(1);
	}
}


void MapData::readData()
{
	string checkType;
	int polygonID;
	float vertexX = -2, vertexY = -2;
	if (file.is_open())
	{
		readSettings();
		vertexData = new vector<vec2>[numOfPolygons + 1];
		while (file >> endLine >> checkType >> polygonID)
		{
			while (file >> vertexX >> vertexY)
			{
				if (checkType == "polygon" || checkType == "Polygon")
				{
					// adding a polygon
					vertexData[polygonID].push_back(vec2(vertexX, vertexY));
					numOfVertices++;
				}
				else {
					// adding the source pts to the beginning of vertexData
					vertexData[0].push_back(vec2(vertexX, vertexY));
					numOfVertices++;
				}
			}
			file.clear();
		}
	}
	else
	{
		cerr << "ERROR: " << filename << " DID NOT OPEN!" << endl;
		exit(1);
	}
}

void MapData::addSourcePoint(vec2 sourcePoint)
{
	vertexData[0].push_back(sourcePoint);
	numOfVertices++;
	setDataArray();
	//setVertexIDs();
}


void MapData::printData()
{
	cout << "-----DOMAIN-----" << endl;
	cout << "Bottomleft: " << "(" << -domain << ", " <<
		-domain << ") " << endl;
	cout << "Topright: " << "(" << domain << ", " <<
		domain << ") " << endl;
	cout << "----------------" << endl;
	cout << endl;
	cout << "-----SOURCES-----" << endl;
	cout << "Number of source points: " << vertexData[0].size() << endl;
	cout << "Points: ";
	if (vertexData[0].size() <= 0)
	{
		cout << "N/A" << endl;
	}
	for (int i = 0; i < vertexData[0].size(); i++)
	{
		cout << "(" << vertexData[0][i].x << ", " <<
			vertexData[0][i].y << ") ";
	}
	cout << endl;
	cout << "----------------" << endl;
	cout << endl;
	cout << "-----POLYGONS-----" << endl;
	cout << "Number of vertices: " << numOfVertices - vertexData[0].size() << endl;
	cout << "Number of polygons: " << numOfPolygons <<	endl;
	for (int i = 1; i <= numOfPolygons; i++)
	{
		cout << "polygon " << i << ": ";
		for (int j = 0; j < vertexData[i].size(); j++)
		{
			cout << "(" << vertexData[i][j].x << ", " <<
				vertexData[i][j].y << ") ";
		}
		cout << endl;
	}
	cout << "---------------" << endl;
}



void MapData::printIndices()
{
	for (int i = 0; i < numOfPolygons; i++)
	{
		cout << "indices of polygon " << i + 1 << ": ";
		for (int j = 0; j < vertexData[i+1].size(); j++)
		{
			cout << int(indices[i][j]) << " ,";
		}
		cout << endl;
	}
}

void MapData::printDataArray(Vertex * dataArray)
{
	for (int i = 0; i < numOfVertices+1; i++)
	{
		cout << "dataArray[" << i << "]: " << "(" << dataArray[i].x 
			<< ", " << dataArray[i].y << ")" << ", dist = " << dataArray[i].distance << ", status = " 
			<< dataArray[i].status << ", parentID = " << dataArray[i].parentID << ", ID = " << dataArray[i].ID << endl;
	}
}

void MapData::setDataArray()
{
	int index = 1;
	dataArray = new Vertex[numOfVertices+1];
	dataArray[0] = { -2, -2, -1., -1, 1., -1. };
	for (int i = 0; i < vertexData[0].size(); i++)
	{
		dataArray[index] = { vertexData[0][i].x, vertexData[0][i].y, 0., float(index), 1., float(index)};
		index++;
	}
	for (int i = 1; i <= numOfPolygons; i++)
	{
		for (int j = 0; j < vertexData[i].size(); j++)
		{
			// distance is practically infinite for obstacle vertices
			// status < 0 for obstacle vertex
			// parent index = -1 b/c its undefined
			//dataArray[index] = { vertexData[i][j].x / domain, vertexData[i][j].y / domain, numeric_limits<float>::max(), -1., -1., float(index)};
			dataArray[index] = { vertexData[i][j].x / domain, vertexData[i][j].y / domain, 1024, -1., -1., float(index)};
			
			index++;
		}
	}
}


void MapData::setIndices()
{
	int index = 0;
	indices = new GLuint*[numOfPolygons];
	for (int i = 0; i < numOfPolygons; i++)
	{
		indices[i] = new GLuint[vertexData[i+1].size()];
		for (int j = 0; j < vertexData[i+1].size(); j++)
		{
			indices[i][j] = GLuint(index);
			index++;
		}
	}
}

vector<vec2>* MapData::getVertexData()
{
	return vertexData;
}

Vertex * MapData::getDataArray()
{
	return dataArray;
}

GLuint ** MapData::getIndices()
{
	return indices;
}

vec2 MapData::getInitDomain()
{
	return vec2(domain, domain);
}

int MapData::getSizeOfDataArray()
{
	return numOfVertices+1;
}

int MapData::getNumOfPolygons()
{
	return numOfPolygons;
}
