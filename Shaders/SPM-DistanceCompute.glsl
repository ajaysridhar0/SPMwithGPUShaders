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

layout(std430, binding = 1) buffer
buffer_StencilValues
{
	uint StencilValues[];
};

Vertex gCurr = DataArray[0];

const uint pixelIndex(vec2 mapCoord)
{
	uint x = int(((1023)/ 2) * (mapCoord.x + 1));
	uint y =	int(((1023) / 2) * (mapCoord.y + 1));
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
			else if (i == 2 && j == 2)
			{
				return false;
			}
		}
	}
	if (shadowCounter >= 25)
	{
		return true;
	}
	if (miniShadowCounter == 9)
	{
		return false;
	}
	return false;
}

void main()
{
	int id = int(gl_WorkGroupID.z * gl_WorkGroupSize.y * gl_WorkGroupSize.x +
	             gl_WorkGroupID.y * gl_WorkGroupSize.x +
	             gl_WorkGroupID.x +
	             gl_LocalInvocationIndex);
	//id = int(gl_LocalInvocationIndex);
	//const uint index = );
	//for ( id = 1; id < DataArray.length(); id++)
	//{
		if ( !inShadow(vec2(DataArray[id].x, DataArray[id].y)))
		{
			vec2 p = vec2(DataArray[id].x, DataArray[id].y);
			vec2 pg = vec2(gCurr.x, gCurr.y);
			
			float newDist = distance(p, pg) + gCurr.distance;
	
			if (DataArray[id].distance == 1024. || newDist < DataArray[id].distance)
			{
				DataArray[id].distance = newDist;
				DataArray[id].parentID = DataArray[0].ID;
			}
		}
	//}
}