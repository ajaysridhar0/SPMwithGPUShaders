// SearchComputeShader.glsl
// 1st shader program in the algorithm
#version 430

layout(local_size_x = 1) in;

// DataArray in DataArray
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

layout ( std430, binding = 0 ) buffer
buffer_DataArray
{
	Vertex DataArray[];
};

void main(void)
{
	//float stop = 3.402823e+39;
	int generatorID = -1;
	float generatorDist = -1;
	uint ntotal = DataArray.length();
	
	for(int i = 1; i < ntotal; i++)
	{
		if(DataArray[i].status != 0.)
		{
			if ((DataArray[i].status > 0.)||(generatorID == -1 ||  DataArray[i].distance < generatorDist))
			{
				generatorID = i;
				generatorDist = DataArray[i].distance;
			}
		}
	}
	DataArray[0] = DataArray[generatorID];
	DataArray[generatorID].status = 0.; // EXPANDED
}

