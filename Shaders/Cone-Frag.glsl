//------------------------------
// Cone-FragmentShader.glsl
// the 3rd shader program in the algorithm 
/--------------------------------

#version 430 compatibility
#extension GL_ARB_shader_storage_buffer_object : require

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

layout(shared, binding = 0) buffer_DataArray
{
    Vertex DataArray[];
	// <x, y, dist, index>
}

 layout(pixel_center_integer) in vec4 gl_FragCoord;
 // will give the screen position of the current fragment

in vec4 gColor;
out vec4 fFragColor;

vec4 gCurr = DataArray[0];

uniform sampler2D prevRender; // texture with the previous render pass

void main()
{
	// bool inShadow = is the pixel in the shadow or not?

	ivec2 screenpos = ivec2(gl_FragCoord.xy); // convert fragment position to integers
	vec4 currentValue = texelFetch(prevRender, screenpos, 0); // what is currently stored in this pixel
	fFragColor = currentValue;
	
	vec2 p = normalize(gl_FragCoord);
	vec2 pg = normalize(gCurr.x, gCurr.y); 
	
	// calculate distance to gCurr and add to gCurr's distance
	// this gives us the length of a canidate for shortest path
	float newDist = distance(p, pg) + gCurr.distance;
	
	if(currentValue.z == -1 || newDist < currentValue.z)
	{
		fFragColor = vec4(gCurr.x, gCurr.y, newDist, 1);
			// x used for red comp
			// y used for green comp
			// length of shortest path for blue comp
			// if cone reaches pixel, alpha channel >0
			// sent to framebuffer
	}
}



