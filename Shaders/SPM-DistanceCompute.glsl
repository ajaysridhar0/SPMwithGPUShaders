// DistanceComputeShader.glsl
// the 4th shader in the algorithm

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


uniform uint windowSize[2];

uniform uint stencilValues[10];

const uint pixelIndex(vec2 mapCoord)
{
	uint x = int(((windowSize[0] -1)/ 2) * (mapCoord.x + 1));
	uint y =	int(((windowSize[1] -1) / 2) * (mapCoord.y + 1));
	return y * windowSize[0] + x;
}

void main()
{
	int id = int(gl_WorkGroupID.z * gl_WorkGroupSize.y * gl_WorkGroupSize.x +
	             gl_WorkGroupID.y * gl_WorkGroupSize.x +
	             gl_WorkGroupID.x +
	             gl_LocalInvocationIndex);
	//const uint index = );
	bool inShadow = (stencilValues[pixelIndex(vec2(DataArray[id].x, DataArray[id].y))] == 1);
	if(!inShadow)
	{
		vec2 p = normalize(vec2(DataArray[id].x, DataArray[id].y));
		vec2 pg = normalize(vec2(DataArray[0].x, DataArray[0].y));
		float newDist = distance(p, pg) + DataArray[0].distance;

		if(DataArray[id].distance < newDist)
		{
			DataArray[id].distance = newDist;
			DataArray[id].parentID = DataArray[0].ID;
		}
	}
}