
#version 430 compatibility
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_EXT_gpu_shader4: enable
#extension GL_EXT_geometry_shader4: enable

// uniform float gCurrX ,gCurrY; 

uniform float csvf;
uniform mat4 modelView, projection;

out vec4 gColor;

layout(lines) in;
layout(triangle_strip, max_vertices = 1024) out;

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
	// <x, y, dist, , initIndex>
};

void ProduceVertex(vec4 v)
{
	gl_Position = modelView * projection * vec4(v.xyz, 1.);
	EmitVertex();
}

void main()
{	
	Vertex gCurr = DataArray[0];
	vec4 p1 = gl_PositionIn[0];
	vec4 p2 = gl_PositionIn[1];
	vec4 pm = (p1 + p2) / 2;
	vec4 pg = vec4(gCurr.x, gCurr.y, 0., 0.);

	float dx = p2.x - p1.x;
	float dy = p2.y - p1.y;
	vec4 g = normalize(pm - pg);
	vec4 n = normalize(vec4(dy, -dx, 0., 0.));
	float d = dot(g, n);

	if (d < .1) // obstacle is front facing with gCurr
	{
		vec4 v1 = normalize(p1 - pg);
		vec4 v2 = normalize(p2 - pg);
		vec4 p1s = p1 + csvf * v1;
		vec4 p2s = p2 + csvf * v2;
		vec4 pms = pm + csvf * g;

		// ----check to see if writing to stencil buffer----
		gColor = vec4(0., 0., 5., 1.);
		// -------------------------------

		ProduceVertex(p1s);
		ProduceVertex(p1);
		ProduceVertex(pms);
		ProduceVertex(p2);
		ProduceVertex(p2s);
		EndPrimitive();
	}
}

//vec2 format (insert in main for use and keep declaration of gCurr)------------------------------------------------------------------
//vec2 p1 = vec2(gl_PositionIn[0].xy);
//vec2 p2 = vec2(gl_PositionIn[1].xy);
//vec2 pm = (p1 + p2) / 2;
//vec2 pg = vec2(gCurr.x, gCurr.y);
//
//float dx = p2.x - p1.x;
//float dy = p2.y - p1.y;
//vec2 g = normalize(pm - pg);
//vec2 n = normalize(vec2(dy, -dx));
//float d = dot(g, n);
//
//if (true || d > .1) // obstacle is front facing with gCurr
//{
//	vec2 v1 = normalize(p1 - pg);
//	vec2 v2 = normalize(p2 - pg);
//	vec2 p1s = p1 + csvf * v1;
//	vec2 p2s = p2 + csvf * v2;
//	vec2 pms = pm + csvf * g;
//
//	// ----check to see if writing to stencil buffer----
//	gColor = vec4(0., 0., 5., 1.);
//	// -------------------------------
//
//	ProduceVertex(p1s);
//	ProduceVertex(p1);
//	ProduceVertex(pms);
//	ProduceVertex(p2);
//	ProduceVertex(p2s);
//	EndPrimitive();
//}
//------------------------------------------------------------------------------------