//------------------------------
// Cone-FragmentShader.glsl
// the 3rd shader program in the algorithm 
//--------------------------------

#version 430 compatibility
#extension GL_ARB_shader_storage_buffer_object : require

// DataArray in DataArray
struct Vertex 
{
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


 layout(pixel_center_integer) in vec4 gl_FragCoord;
 // will give the screen position of the current fragment

out vec4 fFragColor;

uniform sampler2D prevRender; // texture with the previous render pass
uniform uint windowSize; // size of viewing window

Vertex gCurr = DataArray[0];
float distFactor = 128.;

const uint pixelIndex(ivec2 pixel)
{
	return uint(pixel.y * 1024 + pixel.x);
}

bool inShadow(ivec2 pixel)
{
	return (StencilValues[pixelIndex(pixel)] == 1);
}



void main()
{
	fFragColor = vec4(0., 1., 1., 1.);
	//ivec2 screenpos = ivec2(gl_FragCoord.xy); // convert fragment position to integers
//vec4 currentValue = texelFetch(prevRender, screenpos, 0); // what is currently stored in this pixel
//fFragColor = currentValue;
//if (!inShadow(screenpos))
//{
//	vec2 p = normalize(gl_FragCoord.xy);
//	vec2 pg = normalize(vec2(gCurr.x, gCurr.y));
//
//	// calculate distance to gCurr and add to gCurr's distance
//	// this gives us the length of a canidate for shortest path
//	float newDist = distance(p, pg) + gCurr.distance;
//
//	if (currentValue.z == -1 || newDist < currentValue.z * distFactor)
//	{
//		fFragColor = vec4(gCurr.x, gCurr.y, newDist / distFactor, 1);
//		fFragColor = vec4(0., 1., 1., 1.);
//		// x used for red comp
//		// y used for green comp
//		// length of shortest path for blue comp
//		// if cone reaches pixel, alpha channel >0
//		// sent to framebuffer
//	}
//}
}



