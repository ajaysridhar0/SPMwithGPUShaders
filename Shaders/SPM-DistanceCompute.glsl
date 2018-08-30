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

layout (std430, binding = 1) buffer
buffer_StencilValues
{
	uint StencilValues[];
};

uniform uint windowSize[2];

const uint pixelIndex(vec2 mapCoord)
{
	uint x = int(((1024 -1)/ 2) * (mapCoord.x + 1));
	uint y =	int(((1024 -1) / 2) * (mapCoord.y + 1));
	return y * 1024 + x;
}

bool inShadow(vec2 mapCoord)
{
	int shadowCounter = 0;
	int miniShadowCounter = 0;
	for (int i = 0; i <= 4; i++)
	{
		for (int j = 0; j <= 4; j ++)
		{
			if (StencilValues[pixelIndex(mapCoord) + (2 - i)*1024 + j - 2] == 1)
			{
				shadowCounter++;
				if (i >= 1 && i <= 3 && j >= 1 && j <=3)
				{
					miniShadowCounter++;
				}
			}
		}
	}
	if (shadowCounter == 25)
	{
		return false;
	}
	if (miniShadowCounter == 9)
	{
		return false;
	}
	return true;
}

void main()
{
	int id = int(gl_WorkGroupID.z * gl_WorkGroupSize.y * gl_WorkGroupSize.x +
	             gl_WorkGroupID.y * gl_WorkGroupSize.x +
	             gl_WorkGroupID.x +
	             gl_LocalInvocationIndex);
	//const uint index = );
	
	if(inShadow(vec2(DataArray[id].x, DataArray[id].y)))
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