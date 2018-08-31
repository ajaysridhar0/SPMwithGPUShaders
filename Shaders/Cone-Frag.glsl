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

// will give the screen position of the current fragment
layout(pixel_center_integer) in vec4 gl_FragCoord;

in vec2 vTexCoord;
out vec4 fFragColor;

uniform sampler2D prevRender; // texture with the previous render pass
uniform uint windowSize; // size of viewing window

Vertex gCurr = DataArray[0];
float distFactor = 1024;

const uint pixelIndex(ivec2 pixel)
{
	return uint(pixel.y * 1024 + pixel.x);
}

bool inShadow(ivec2 pixel)
{
	return (StencilValues[pixelIndex(pixel)] == 1);
}

vec2 normFragCoord()
{
	return vec2(gl_FragCoord.x/512 -1., gl_FragCoord.y/512 -1.);
}

void main()
{
	ivec2 screenpos = ivec2(gl_FragCoord.xy); // convert fragment position to integers
	vec4 currentValue = texture(prevRender, vTexCoord); // what is currently stored in this pixel
	// if nothing else, pass the current value on
	fFragColor = currentValue;
	//fFragColor = vec4(0., 1., 1., 1.);
	if (!inShadow(screenpos))
	{
		//fFragColor = currentValue;
		//fFragColor = vec4(.5, .5, .5, 1.);
		vec2 p = normFragCoord().xy;
		vec2 pg = vec2(gCurr.x, gCurr.y);

		// calculate distance to gCurr and add to gCurr's distance
		// this gives us the length of a canidate for shortest path
		float newDist = (distance(p, pg) + gCurr.distance)/distFactor;
		//fFragColor = vec4(gCurr.x, gCurr.y, newDist / distFactor, 1.);
		if ((currentValue.a == 0. || newDist < (currentValue.z)))
		{
			fFragColor = vec4(gCurr.x, gCurr.y, newDist, 1.);
				// x used for red comp
				// y used for green comp
				// length of shortest path for blue comp
				// if cone reaches pixel, alpha channel >0
				// sent to default framebuffer
		}
	}
}



